## Programming Assignment 1

### Submission Guidelines
To be announced.

#### Submission Periods
- Parts 1 and 2: To be announced.  
- Part 3: To be announced.

### 1. [Using ABC]
(10%)  
(a) Use [BLIF manual](http://www.eecs.berkeley.edu/~alanmi/publications/other/blif.pdf) to create a BLIF file representing a four-bit adder.  
(b) Perform the following steps to practice using ABC:
 1. read the BLIF file into ABC (command `read`)
 2. check statistics (command `print_stats`)
 3. visualize the network structure (command `show`)
 4. convert to AIG (command `strash`)
 5. visualize the AIG (command `show`)
 6. convert to BDD (command `collapse`)
 7. visualize the BDD (command `show_bdd -g`)


### 2. [ABC Boolean Function Representations]
(10%)  
In ABC there are different ways to represent Boolean functions.  
(a) Compare the following differences with the four-bit adder example.  
1. logic network in AIG (by command `aig`) vs.
structurally hashed AIG (by command `strash`)
2. logic network in BDD (by command `bdd`) vs.
collapsed BDD (by command `collapse`)

(b) Given a structurally hashed AIG, find a sequence of ABC command(s) to covert it to a logic network with node function expressed in sum-of-products (SOP).

#### Items to turn in for Part 1 and 2:
 - The four-bit adder BLIF file.
 - A PDF report containing:
    - results of `show` and `show_bdd -g` after step 3,5,7 in Part 1
    - answers of question (a),(b) in Part 2. 

### 3. [Programming ABC]
(80%)  
To be announced.
