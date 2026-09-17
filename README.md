*This project has been created as part of the 42 curriculum by mhend.*

# Codexion

## Description

Codexion is a concurrency simulation written in C with POSIX threads. Coders
sit around a circular co-working hub and share a pool of USB dongles: there are
exactly as many dongles as coders, and each coder needs the two dongles next to
them — one on the left, one on the right — to compile their quantum code.

Each coder alternates between three exclusive phases: **compiling** (holding
two dongles), **debugging** and **refactoring**. A coder who does not *start*
compiling within `time_to_burnout` milliseconds of their previous compile start
burns out, and the simulation stops immediately. The simulation also stops when
every coder has compiled at least `number_of_compiles_required` times.

The interesting part is not the phases, it is the arbitration:

* every dongle keeps a **waiting queue implemented as a binary heap** (a hand
  written priority queue, no standard library container),
* dongles are granted according to a **scheduling policy** chosen on the
  command line: `fifo` (arrival order) or `edf` (earliest burnout deadline
  first, deadline = `last_compile_start + time_to_burnout`),
* a released dongle stays unavailable for `dongle_cooldown` milliseconds,
* a dedicated **monitor thread** detects burnout and stops the simulation
  within 10 ms of the actual deadline.

## Instructions

```sh
make                # builds ./codexion with -Wall -Wextra -Werror -pthread
make clean          # removes the object files
make fclean         # removes the object files and the binary
make re             # fclean + all
```

Run it with **eight mandatory arguments**:

```
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument | Meaning |
| --- | --- |
| `number_of_coders` | number of coders, and number of dongles (1 … 500) |
| `time_to_burnout` | ms allowed between two compile starts (≥ 1) |
| `time_to_compile` | ms spent compiling, holding both dongles |
| `time_to_debug` | ms spent debugging |
| `time_to_refactor` | ms spent refactoring, then the coder asks for dongles again |
| `number_of_compiles_required` | the run stops once every coder reached this count |
| `dongle_cooldown` | ms during which a released dongle cannot be taken again |
| `scheduler` | exactly `fifo` or `edf` |

Every numeric argument must be a non-negative integer that fits in an `int`;
anything else (a negative number, a float, a letter, a missing argument, an
unknown scheduler) is rejected with a usage message and exit code `1`.

### Usage examples

```sh
./codexion 5 800 200 100 100 10 0 edf     # nobody should burn out
./codexion 4 410 200 100 100 10 0 edf     # tight, but feasible
./codexion 1 800 200 200 200 5 0 fifo     # a single coder: one dongle, burnout
./codexion 5 800 200 100 100 5 100 fifo   # with a 100 ms dongle cooldown
./codexion 4 310 200 100 100 100 0 edf    # infeasible on purpose: burnout
```

Output format, one line per state change, timestamps in ms since the start:

```
0 1 has taken a dongle
0 1 is compiling
200 1 is debugging
300 1 is refactoring
1204 3 burned out
```

Notes on two edge cases:

* `number_of_compiles_required = 0` is satisfied before anything happens, so
  the simulation stops immediately and prints nothing. Pass a large value to
  run "until somebody burns out".
* with a single coder there is a single dongle: the coder takes it, cannot
  compile with one dongle, and burns out — this is the expected behaviour.

### Choosing feasible parameters

Coders sit in a ring, so two coders can only compile at the same time when they
are not neighbours: at most `n / 2` coders compile concurrently (exactly one
when `n <= 3`). A rough feasibility rule for `n >= 4`:

```
time_to_burnout  >  max(time_to_compile + time_to_debug + time_to_refactor,
                        3 * (time_to_compile + dongle_cooldown))
```

The first term is the coder's own cycle, the second is the time needed for the
ring to hand the dongles around. Parameters close to that bound are knife-edge:
they may complete or burn out depending on the load of the machine.

## Blocking cases handled

### Deadlock — the four Coffman conditions

| Condition | How it is handled |
| --- | --- |
| Mutual exclusion | Kept: a dongle really is exclusive, its state is guarded by its own `pthread_mutex_t`. |
| **Hold and wait** | **Broken.** A coder never holds one dongle while waiting for the other. It registers a single request in both queues and the arbiter hands it *both* dongles in one critical section, or none at all. |
| No preemption | Kept: a dongle is released by its holder only, at the end of the compile. |
| **Circular wait** | **Broken twice.** The two dongle mutexes are always locked in increasing index order, and one request carries the *same* priority key in both queues, so the "waits for" relation follows a total order and cannot contain a cycle. |

The classic failure this avoids: coder N grabs its left dongle, waits for its
right one, and every coder does the same. Here the arbitration is all-or-
nothing, so that state is unreachable.

### Starvation

Blocking a resource for a waiting high priority request is what prevents
starvation, but doing it unconditionally leaves dongles idle and collapses
throughput to one compile at a time. The arbiter therefore uses **bounded,
deadline aware backfilling**:

* requests are examined in scheduler order (`fifo` or `edf`);
* a request that cannot be served right now is *blocked*, and a lower priority
  request may be served before it (the dongle is not wasted);
* every overtake is charged to the blocked request (`bypass` counter). Once it
  has spent its budget (`BYPASS_LIMIT`, in `include/codexion.h`), its two
  dongles are closed to everybody else, which bounds its waiting time;
* an overtake is also refused as soon as it would no longer leave the blocked
  request enough time to reach its own burnout deadline, that is when
  `now + time_to_compile + dongle_cooldown > deadline`.

Consequence: the most urgent request is never blocked by a reservation it does
not outrank, so it is served as soon as its dongles are physically free — at
most one compile plus one cooldown later. Setting `BYPASS_LIMIT` to `0` gives a
strictly ordered arbiter and shows the throughput cost of the naive policy.

### Lost wake-ups and the "thundering herd"

A coder waiting for a *pair* of dongles cannot sleep on a single dongle's
condition variable, since either dongle may become available first. Each coder
therefore owns a small event channel (`wake_lock` + `wake_cond` + `granted`
flag). The decision is taken by the arbiter and *handed over*: the coder is
woken only when its two dongles are actually its own, never to re-check and go
back to sleep. The flag is set under the same mutex the sleeper uses, so a
grant that happens between the test and the `pthread_cond_wait` cannot be lost.

### Dongle cooldown

`dongle_release()` stamps `free_at = now + dongle_cooldown`. A dongle is
considered free only when `holder == 0 && now >= free_at`, so nobody can pick
it up early. The monitor re-runs an arbitration pass every 300 µs, which is
what wakes the simulation up when a cooldown expires (no thread is ever left
sleeping on a timer that has already elapsed).

### Precise burnout detection

The monitor thread is the only one allowed to declare a burnout. It checks
every coder every 300 µs, so the `burned out` line is printed roughly 1 ms
after the deadline, well inside the 10 ms required. `last_compile_start` is
written by the coder and read by the monitor under `state_lock`, so the check
never races with a compile that is starting at the same instant.

### Log serialization

`log_state()` takes `log_lock` before printing, so two lines can never be
interleaved. It also re-reads the `stopped` flag while holding the lock: no
state message can be printed after `burned out`, which is the usual "one line
too many" bug at the end of a run.

## Thread synchronization mechanisms

### Threads

* `n` **coder threads** (`coder_routine`): request → compile → debug →
  refactor, forever.
* one **monitor thread** (`monitor_routine`): detects burnout, detects the
  completion of `number_of_compiles_required`, re-arbitrates on cooldown
  expiry, and wakes everybody when the simulation ends.
* the **main thread** creates them, joins them, and frees everything.

No global variable is used: a single `t_sim` lives on `main`'s stack and each
thread receives a pointer to its own `t_coder`, which points back to `t_sim`.

### Mutexes and condition variables

| Primitive | Protects | Notes |
| --- | --- | --- |
| `dongle.lock` (one per dongle, `pthread_mutex_t`) | `holder`, `free_at` and the waiting queue of that dongle | required by the subject: every dongle owns its mutex |
| `sim.arbiter` (`pthread_mutex_t`) | serializes whole arbitration passes | guarantees that a pass sees a consistent snapshot of the table |
| `coder.wake_lock` + `coder.wake_cond` + `coder.granted` | the hand-over of a decision to one coder | a custom event: `wake_grant()` signals, `wake_wait_grant()` waits |
| `sim.state_lock` (`pthread_mutex_t`) | `stopped`, `seq`, `compiles`, `last_start` | shared by the coders and the monitor |
| `sim.log_lock` (`pthread_mutex_t`) | `stdout` | one line at a time |

Locks are always taken in the order

```
arbiter  →  dongle.lock  →  wake_lock  →  state_lock          (log_lock → state_lock)
```

and two dongle mutexes are always taken in increasing index order. Since the
order is a strict hierarchy, no lock cycle — hence no deadlock — is possible
inside the synchronization layer itself.

### How race conditions are prevented

* **Two coders taking the same dongle.** `holder` is only ever written inside
  `dongle_hold()` / `dongle_release()`, both under that dongle's mutex, and
  both reachable only from an arbitration pass holding `sim.arbiter`.
* **A request seen half registered.** `request_enqueue()` pushes into both
  queues *and* runs the arbitration pass inside a single `arbiter` section, so
  no pass can ever grant a request that only exists in one queue.
* **Compile counters and deadlines.** `compiles` and `last_start` are written
  by the coder and read by the monitor under `state_lock`; the EDF deadline of
  a request is computed once, under the same mutex, and copied into the request
  so that both queues sort it identically.
* **Stopping the simulation.** `stopped` is only written under `state_lock`.
  Sleeping coders poll it (`sim_sleep`), waiting coders re-read it inside their
  `pthread_cond_wait` predicate, and the monitor signals every event channel
  after setting it, so no thread can stay blocked after the end.

### Thread-safe communication between the coders and the monitor

The coders never talk to each other, as the subject requires. They publish
their state (`compiles`, `last_start`) under `state_lock`; the monitor reads
it, decides, and communicates back in exactly two ways: the `stopped` flag and
the per-coder event channels. There is no shared buffer and no polling of one
coder by another.

### Priority queue

`src/pqueue*.c` implements a binary min-heap of requests (`pq_push`, `pq_pop`,
`pq_remove`, `pq_sift_up`, `pq_sift_down`), ordered by:

* `fifo`: request sequence number;
* `edf`: burnout deadline, ties broken by sequence number.

The sequence number is a global counter incremented once per request, so the
order is total and fully deterministic even when two deadlines are equal. The
same heap type is reused as the scratch queue that the arbiter fills with the
pending requests of a pass.

## Project layout

```
Makefile
include/codexion.h
src/main.c          argument entry point
src/parse.c         strict parsing and validation of the eight arguments
src/time.c          gettimeofday helpers, interruptible sleep
src/log.c           serialized logging, stopped flag
src/pqueue*.c       binary heap used as the dongle waiting queues
src/wake.c          per-coder event channel
src/dongle*.c       dongle state: queue, holder, cooldown
src/arbiter.c       the arbitration pass (fifo / edf, backfilling)
src/request.c       coder side protocol: request, wait, release
src/coder.c         compile / debug / refactor loop
src/monitor.c       burnout detection, end of simulation
src/init.c, run.c   setup, thread creation and joining, cleanup
```

## Resources

* medium, geeksforgeeks, posix standards, youtube, peer learning, reddit, github