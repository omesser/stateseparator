# Quantum State separator    

## Overview
**The state separator implements a probabilistic numerical algorithm that checks whether a given multi-Qudits
state is separable or entangled.**
 
The algorithm is based on *"Geometrical aspects of entanglement"*, Physical review A 74, 012313 (2006),
by Jon Magne Leinaas, Jan Myrheim and Eirik Ovrum.

The state separator also performs a *Peres-Horodecki* partial-transpose test on the matrix. The *Peres-Horodecki* test is 
always a necessary condition for separability, and is also sufficient for `2 X 2` and `2 X 3` sized systems.

The state separator will find the nearest separable state and its decomposition to pure product states and give 
the	distance from the target matrix.

Simply copy the density matrix to be tested to the main program window,
enter the Qudits (particles) number and dimension (i.e. `2 2` for 2-Qubits) and hit the `separate` button on the right.

If the input state fails the *Peres-Horodecki* test - it will be declared `entangled (Peres Test)`.
Otherwise:
- If the target distance from the given matrix was reached (default `0.5E-13`) - it will be declared `separable`.
- else, if the distance is less than `0.0005` - it will be declared `might be entangled`.
- else - it is declared `most likely entangled`.

In any case, the full details will be displayed right below the result message.

For more details about the algorithm used, read [main algorithm](##main-algorithm), or the appropriate article.

We hope you find this tool useful. For bugs and issues please contact us (see contact page).


## Usage
### Qudits number and dimension window

    Format: [Integer] [white_space] [Integer][white_space] and so on....

Example - 2-Qubit: `"2 2"` (Two particles, of size 2 each)

Example - Qubit+Qutrit: `"2 3"` (Two particles, of sizes 2 and 3 appropriately)

Note: The product of the Qudits' dimension must be the same as the dimension/order of the input matrix.

### <a name="main-window">Main window - matrix & output

*Paste your matrix here!*
The system does basic input check, (#rows=#cols, only numerical data input, etc. ). 
The matrix elements should be delimited by one or more white-spaces, and newline indicates a new row.
The system ignores its own results after the input matrix, so you can hit "separate" multiple 
times without cleaning the input window, and the system will append the new results to the bottom of the main window.


Matrix-element format examples: `+2.3-i0.2, 3, 5i, -0.2i, 0.6i-3.1, i+0.1`

For exact formatting options see [appendix](##appendix)

#### Additional restrictions
The Matrix given is checked to be Hermitian, semi-definite positive, 
and to have unit-trace.	If the conditions aren't met (to 3rd decimal digit only), a warning message will be displayed. 
Please note this will affect the relevance of the output, as the algorithm can only reach a hermitian, semi-definite-positive!

### Optional parameters
#### Target distance

Sets the minimal distance between the input matrix and its best separable approximant, 
such that they are considered equal.
This is set by default to `0.5E-13`, and can be overridden with any positive value.

#### Minimum weight per state

The algorithm weeds out from the mixed-states approximation, pure-states with a coefficient 
below a minimum threshold. This sets this threshold, and it should be a positive weight
The default value is set to `0`.

#### Target number of states

The target number of product states that comprise the mixed-state-approximation. On default
this is set to `N^2`, but can be overridden to a manual value to the user's choice.

#### Output precision

Determines how many decimal digits are going to be displayed in the output.

Chose between `3`,`6`,`9`,`12`,`15`. Default is `3`.
			
Note: This only sets output-display accuracy, and does NOT affect the floating point calculation accuracy of the system.

#### Accuracy boost mode

A toggle that determines the values of 2 heuristics in the algorithm.

Without the accuracy boost (default):

    M = N^2 + N
    R = 100

And in choosing the accuracy boost mode will set:

    M = N^3
    R = 1000

Note: for further details read about the algorithm time complexity in [time complexity](##time-complexity)

## Main algorithm

The algorithm implemented is an iterative search for a close approximation to the input matrix WITHIN
the separable matrices subspace. 

Starting from the maximally mixed state, the system iteratively
adds pure states to the mixed-state approximation, where each pure state is generated to 
maximize the projection of the distance vector between the current best-approximation matrix, 
and the original input matrix, on the separable matrices subspace.


The main iteration repeats until either a target distance to original matrix is reached 
(see [target distance](####target-distance) or until a number of iterations is reached. The heuristic for the maximum
number of main iterations is `(target-number-of-states)^2` (see [target number of states](####target-number-of-states)).

Minimization of the distance between the current best approximation mixed-state matrix, and the original, is 
reached implementing a quadratic-programming optimization approach. The equation system is solved efficiently using 
Eigen's LDLT Cholesky's decomposition.

The pure states collection from which the approximation is constructed is optimized by discarding 
states with probability below the "Minimum probability per state" threshold 
(see [minimum weight per state](####minimum-weight-per-state)).

In each main iteration, the construction of the best candidate pure state to mix-in to our
approximation matrix is also an iterative numerical process on it's own.


Here we use distance between the original matrix and our last approximation to generate a tensor-product pure-state,
using a compound adjoint Eigen problem (for each particle). 

This process refines the pure state until a maximal Eigen value is reached consistently for all particles, 
or a heuristic is reached (`500` iterations currently).

For more background and details , see the original paper: *"Geometrical aspects of entanglement"*,
Physical review A 74, 012313 (2006).
by Jon Magne Leinaas, Jan Myrheim and Eirik Ovrum.


## Accuracy
#### Input accuracy

The input accuracy is very loosely restricted. The floating point calculation of a the data representation
is limited to 14 significant digits, so entering more than this	is redundant. It is redundant to enter more digits than 
the output format (see [output accuracy](####output-accuracy)), or than the accuracy of the algorithm itself - 
6-7 significant digits (see [algorithm accuracy](####algorithm-accuracy)).

#### Output accuracy

This refers to the manual setting of the display format of the output. See [output percision](####output-percision). 
Setting this will **NOT** affect the calculation time or accuracy (see [algorithm accuracy](####algorithm-accuracy)), 
but simply **the number of decimal digits displayed**.

#### <a name="####algorithm-accuracy">Algorithm/Calculation accuracy

The numerical nature of the algorithm makes defining it's accuracy a not-well-defined problem.
The representation is only limited to the floating point error (`E-19`) but looking for pure states and tracing out
the ideal pure states in each step is a numerical approximation. The choice of heuristics was made such that a good
enough accuracy was reached, while not allowing the algorithm to take more than a few seconds on typically sized 
input matrices.


As a measure of accuracy, we consider differentiating between separable and non-separable Werner states, 
parametrized by `q`:

    W = q*I + (1-q)*B

Where `I` is the normalized `4x4` mixed state, and `B` is any of the Bell states.
For q &lt 1/3 the state is separable, while for q &gt, entangled.
In it's current optimization settings, using the [*accuracy boost mode*](####accuracy-boost-mode), the system will find a 
separable approximation for `W`, within less than `0.5E-13`, for `q=0.3332` (or less, of course). This means that for 
this marginal type of matrix, we achieve accuracy of `0.5E-4` in the parameter `q`.


## Time complexity

Worst case time complexity stands at:

    O(R * n * M * N^3)

Where:

    R = traceOut refinement steps
    M = maximum main iteration steps defined. see accuracy boost sec 2.3.5.
    N = order of the input matrix
    n = number of particles 

Details:


The initial Peres test, test's a single particle partial transpose per each particle.
Total time complexity for this phase stands at `O(n * N^2)` and is not the limiting value.

The main algorithm uses several heuristics to limit it's computation time.
First, the main iteration (how many time do we search for an extra pure-state to mix-in
with our approximation) is limited by M depending on whether [accuracy boost mode](####accuracy-boost-mode)
is chosen, if no other breaking condition is reached. 

Each such iteration requires that we build a new pure-state, which is done in several 
iterations, each refining the projection vector tensor products.
The number of iterations here is limited and maximum value is set by the [accuracy boost mode](####accuracy-boost-mode).
In which we iterate over the particles (n), projecting the traced out parts over the particle's subspace `O(N^2)`,
and solving to get the maximal Eigen-value `O(N^3)`.
After the new pure-state is built we mix it in to our approximation and solve another 
Eigen problem (QP problem) to find the correct coefficients for our different pure-states
in `O(N^3)`.
finally we perform a rudimentary selection sort to provide the users with the states
sorted in a descending order according to their respective probability.
This takes an additional `O(targetNumStates * log(targetNumStates))` which is 
defaulted to `O(N^3)` unless determined otherwise by user input.
So Total Time complexity stands at:
    
    O( (R * n * [N^2 + N^3 + N^3])  + N^3 ) &lt= O(R * n * N^3)


## Version and release date

The **State-separator** started in 2012 as an under-graduate student project at the [Physics department of the 
Technion - Israel institute of technology, Haifa, Israel](https://phys.technion.ac.il/en/),
by [Naftaly Shalev](https://www.linkedin.com/in/naftaly-shalev-36b53711a) and completed in 2014 
by [Oded Messer](https://github.com/omesser) under the guidance of 
[J. Avron](https://phsites.technion.ac.il/avron/).  

It provide students, researchers and  quantum information enthusiasts an easy to use tool to 
test for separability of quantum systems.		

## Licensing and disclaimer

The system core logic is written in c++ code using Eigen free-library, and simple html and php layer for web access.

The State Separator is distributed as OSS under [the GPL-3.0 License](LICENSE)

**You may use this program and distribute it freely.**


The state-separator program contains only original code, and code from the Eigen library. Eigen is a free 
software, distributed under the LGPL3+ license (and MPL2 in later versions).
[Eigen license page](http://eigen.tuxfamily.org/index.php?title=Main_Page#License)

Feedback and bug reports are welcome [contact page](http://phweb.technion.ac.il/~stateseparator/contact.html)

Neither the Technion nor the authors are responsible to any outcome 
the usage of the system may lead to.

## Credit and Thanks

The system was conceived by [J. Avron](https://phsites.technion.ac.il/avron/), and the first version of the 
code was developed and written by [Naftaly Shalev](https://www.linkedin.com/in/naftaly-shalev-36b53711a).
2nd code iteration, algorithm improvements, testing and write-up of documentation and menus was 
done by [Oded Messer](https://github.com/omesser).

We thank [Dr. Oded Kenneth](https://phys.technion.ac.il/en/people/person/295) for his wise mathematical 
council in several critical stages of the project.
   
## <a name="##appendix">Appendix: Matrix-element format
            
The matrix elements are expected in the following formats:
		
    [+/-][real_part][+/-][i][img_part]   		Example: +2.3-i0.2
    [+/-][real_part][+/-][img_part][i]   		Example: -2.3-0.2i
    [real_part][+/-][i][img_part]   		Example:  2.3-i0.2
    [real_part][+/-][img_part][i]   	 	Example:  2.3-0.2i
    [+/-][i][img_part][+/-][real_part]	        Example: +i2.3-0.2
    [+/-][img_part][i][+/-][real_part] 	        Example: -2.3i-0.2
    [i][img_part][+/-][real_part]  		 	Example: i2.3-0.2
    [img_part][i][+/-][real_part]   		Example: +2.3i+0.2
    [+/-][real_part] 				Example: -2.3
    [real_part]   					Example:  2.3
    [+/-][img_part][i]   				Example: +2.3i
    [+/-][i][img_part]   				Example: -i2.3
    [img_part][i]   		 		Example:  2.3i
    [i][img_part]  	 				Example: i2.3


- Note: `i` without number will be interpreted as: `1*i` i.e. `3+i = 3+1i`


*Developed at the Technion - Israel Institute of Technology, Haifa, Israel*