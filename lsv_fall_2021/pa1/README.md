## Programming Assignment 1

### Submission
- Parts 1 and 2: 2021/10/08 23:59  (by online submission on CEIBA) 
- Part 3: 2021/10/22 23:59 (by online submission on CEIBA)

### 1. [Using ABC]
(10%)  
(a) Use [BLIF manual](http://www.eecs.berkeley.edu/~alanmi/publications/other/blif.pdf) to create a BLIF file representing a four-number serial adder.  
(b) Perform the following steps to practice using ABC:
 1. read the BLIF file into ABC (command `read`)
 2. check statistics (command `print_stats`)
 3. visualize the network structure (command `show`)
 4. convert to AIG (command `strash`)
 5. visualize the AIG (command `show`)
 6. convert to BDD (command `collapse`)
 7. visualize the BDD (command `show_bdd -g`; note that `show_bdd` only shows the first PO; option `-g` can be applied to show all POs ) 


### 2. [ABC Boolean Function Representations]
(10%)  
In ABC there are different ways to represent Boolean functions.  
(a) Compare the following differences with the four-number serial adder example.  
1. logic network in AIG (by command `aig`) vs.
structurally hashed AIG (by command `strash`)
2. logic network in BDD (by command `bdd`) vs.
collapsed BDD (by command `collapse`)

(b) Given a structurally hashed AIG, find a sequence of ABC command(s) to covert it to a logic network with node function expressed in sum-of-products (SOP).

#### Items to turn in for Part 1 and 2:
 1. The BLIF file.
 2. A PDF report containing:
    - results of `show` and `show_bdd -g` after step 3,5,7 in Part 1
    - answers of question (a),(b) in Part 2. 

Put the files into a folder named <your student ID> (e.g. r10943108), and compress it into <your student ID>.tgz by
```
tar -czvf <your student ID>.tgz <your student ID>
```
Submit the tgz through CEIBA.

### 3. [Programming ABC]
(80%)  
To be announced.
