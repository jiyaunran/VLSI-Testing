# PODEM – ATPG for Stuck-at and Bridging Faults

This project consists of six main assignments, forming a complete Automatic Test Pattern Generation (ATPG) flow based on the PODEM algorithm. It supports both **stuck-at faults** and **bridging faults (AND/OR bridging)**.

### 📘 Features

1. **Circuit Analysis**  
   Parse the input circuit and output the number of inputs/outputs and supported gate types.

2. **Path Finding**  
   Given a start and end gate, list all possible logic paths between them.

3. **Pattern Generation**  
   Generate a specified number of random input patterns based on the parsed circuit.

4. **Stuck-at Fault Listing**  
   Enumerate all possible single stuck-at faults in the circuit.

5. **Bridging Fault Listing**  
   Identify and list possible AND-bridging and OR-bridging faults based on logic structure.

6. **Fault Simulation**  
   Simulate test patterns for stuck-at faults and extended simulation support for bridging faults.

---

### 🔧 Key Modified Files

- `main.cc`: Integrates all assignments and command parsing.
- `ATPG.cc`: Implements bridging fault list generation logic.
- `fault.h`: Defines the structure for bridging faults.
- `fault.cc`: Adds bridging fault simulation logic.
- `circuit.h`: Includes new circuit analysis and utility functions.

---

### 📌 Notes
- Gate types and circuit format follow the standard ISCAS-85 style.
- Bridging fault logic is based on voltage dominance modeling (pull-to-zero / pull-to-VDD).

---