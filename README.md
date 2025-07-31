# Convex Hull Server – Assignment 3 (Semester B 2025)

This project implements a **multi-threaded convex hull server**, developed step-by-step across ten phases as part of an Operating Systems programming assignment.

---

## Authors:
Aviv Neeman, Gal Maymon

## ✅ Step-by-Step Overview

### **Step 1: Basic CH Program**
A simple program receives a list of 2D points (float format, `x y` or `x,y`), computes their **Convex Hull**, and prints the **area** using the `compute_convex_hull()` and `compute_area()` functions.

---

### **Step 2: Profiling**
We profiled three containers – `std::vector`, `std::list`, and `std::deque` – to compare their efficiency for Convex Hull computation:
- Each container was seeded with the same 1,000,000 random points using a fixed `srand` seed.
- Five executions were averaged to minimize OS noise.
- Results showed `std::vector` performed best with our CH logic.

---

### **Step 3: Basic Interaction**
We added a command interface to interactively:
- `Newgraph n`: Accept `n` points for a new graph.
- `CH`: Compute the convex hull of the current graph.
- `Newpoint x,y`: Add a new point.
- `Removepoint x,y`: Remove a point.

Commands are parsed from `stdin`. Each ends with LF (`\n`).

---

### **Step 4: Multi-user Server**
Created a TCP server (`port 9034`) allowing **multiple clients** to send commands concurrently.
- The graph is a shared resource.
- Commands from other clients are blocked while another is inputting points after `Newgraph <n>` command or `CH` 
  to ensure uninterrupted client experience.

---

### **Step 5: Reactor Template**
We created a generic `Reactor<Callback>` template using `select()` to associate FDs with callbacks.
- Generic design allows plugging in any callback.
- Callbacks are triggered when corresponding file descriptors become readable.

---

### **Step 6: Server Using Reactor**
Integrated the reactor template with the multi-user server (step 4).
- Each client FD is handled using a callback.
- The logic for graph state remains the same.

---

### **Step 7: Thread-per-Client Server**
Re-implemented the server using **one thread per client**.
- Accept thread + client handler threads.
- Graph is protected via `mutex` given multi threads are now used.
- Added detailed logging for each client action on the server side.

---

### **Step 8: Proactor Template**
A `Proactor` abstraction was added:
- Accepts incoming clients and delegates each new connection to a handler thread.
- Allows clean separation between accepting and processing connections.

---

### **Step 9: Server Using Proactor**
Built full server using the `Proactor` template.
- Maintains the exact same graph logic as Step 7 (including restrictions when another client is mid-`Newgraph`).
- Added full logging of every client action on the server side.

---

### **Step 10: Producer-Consumer (10 Points)**
Added a **monitoring thread** that wakes using a **POSIX condition variable (`pthread_cond_t`)**:
- Triggered **only after a `CH` command** finishes.
- If the CH area becomes ≥100 and wasn't before, prints:
  ```
  At Least 100 units belongs to CH
  ```
- If CH area drops below 100 after previously exceeding it, prints:
  ```
  At Least 100 units no longer belongs to CH
  ```

All graph logic from Step 9 (including `Newgraph`, `Newpoint`, `Removepoint` rules) is preserved.

---

## 🛠️ Compilation

Use `make step<n>` (n being the step from 1-10, excluding 5 and 8 which are templates), to compile and run.  
If it's a server, open a different terminal and enter `nc localhost 9034` to connect to the server as a client.

---

## 🚀 Execution

Example run for Step 4 (from the main directory, relevant for steps 4+):
```bash
make step4
```

Then in another terminal:
```bash
nc localhost 9034
Newgraph 3
1,1
2,2
3,3
CH
```

---

## 🔐 Synchronization Notes

- Shared access to the `Graph` is protected by `std::mutex` whenever multi-threading is used.
- `graph_busy` ensures mutual exclusion for operations requiring full graph access (e.g., `Newgraph`).
- Threads check ownership using `current_owner_fd` to allow fair access.
- In Step 10, `condition_variable` is used for the CH monitoring thread to wake only after `CH` commands.

---

## 📦 Contents

- `step<n>_.../`: Source code for each step.
- `common/include/`: Shared headers (`Graph`, `Point`, `CH` logic).
- `Makefile`: Compiles the required server or step binary.
- `README.md`: This file.

---

Made for Operating Systems course
