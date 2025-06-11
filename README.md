# Convex Hull Server – Assignment 3 (Semester B 2024–2025)

This project implements a **multi-threaded convex hull server**, developed step-by-step across ten phases as part of a systems programming assignment.

---

## ✅ Step-by-Step Overview

### **Step 1: Basic CH Program**
A simple program receives a list of 2D points (float format, `x y` or `x,y`), computes their **Convex Hull**, and prints the **area** using the `compute_convex_hull()` and `compute_area()` functions.

---

### **Step 2: Profiling **
We profiled three containers – `std::vector`, `std::list`, and `std::deque` – to compare their efficiency for Convex Hull computation:
- Each container was seeded with the same 1,000,000 random points using a fixed `srand` seed.
- Five executions were averaged to minimize OS noise.
- Results showed `std::vector` performed best with our CH logic.

---

### **Step 3: Basic Interaction **
We added a command interface to interactively:
- `Newgraph n`: Accept `n` points for a new graph.
- `CH`: Compute the convex hull of the current graph.
- `Newpoint x,y`: Add a new point.
- `Removepoint x,y`: Remove a point.

Commands are parsed from `stdin`. Each ends with LF `
`.

---

### **Step 4: Multi-user Server **
Created a TCP server (`port 9034`) allowing **multiple clients** to send commands concurrently.
- The graph is a shared resource.
- Commands from other clients are blocked while another is inputting points after `Newgraph <n>` command
  to ensure uninterrupted client experience.

---

### **Step 5: Reactor Template **
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

---

### **Step 8: Proactor Template (10 Points)**
A `Proactor` abstraction was added:
- Accepts incoming clients.
- Delegates each new connection to a handler thread.

---

### **Step 9: Server Using Proactor **
Built full server using the `Proactor` template.
- Accepts clients and handles I/O asynchronously.
---

### **Step 10: Producer-Consumer (10 Points)**
Added a **monitoring thread** that wakes using a **POSIX condition variable (`pthread_cond_t`)**:
- Triggered after a `CH` command finishes.
- If the CH area becomes ≥100 and wasn't before, prints:
  ```
  At Least 100 units belongs to CH
  ```
- If CH area drops below 100 after previously exceeding it, prints:
  ```
  At Least 100 units no longer belongs to CH
  ```

---

## 🛠️ Compilation

Use `make` to build and run each step in its corresponding directory.

## 🚀 Execution

Example run for Step 4+:
```bash
make
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

## 🔐 Synchronization Notes

- Shared access to the `Graph` is protected by `std::mutex`.
- `graph_busy` ensures mutual exclusion for operations requiring full graph access (e.g., `Newgraph`, `CH`).
- Threads check ownership using `current_owner_fd` to allow fair access.

---

## 📦 Contents

- `src/`: Source code for each step.
- `include/`: Shared headers (`Graph`, `Point`, `CH` logic).
- `Makefile`: Compiles the required server or step binary.
- `README.md`: This file.

---

## 🧠 Notes

- Followed the POSIX thread model with proper condition signaling.
- Ensured correctness under concurrent client access.
- Profiled container performance with fairness and repeatability.

---

Made with ❤️ for Operating Systems course @ Bar-Ilan University.