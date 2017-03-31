# ME-E4100 Advanced Computer Graphics, Spring 2017
# Lehtinen / Härkönen, Ollikainen
#
# Assignment 2: Radiosity

Student name: Valtteri Wahlroos
Student number: 357096

# Which parts of the assignment did you complete? Mark them 'done'.
# You can also mark non-completed parts as 'attempted' if you spent a fair amount of
# effort on them. If you do, explain the work you did in the problems/bugs section
# and leave your 'attempt' code in place (commented out if necessary) so we can see it.

R1 Integrate old raytracing code (1p): done
            R2 Area light source (3p): done
                    R3 Radiosity (6p): done

# Did you do any extra credit work?

(Describe what you did and, if there was a substantial amount of work involved, how you did it. If your extra features are interactive or can be toggled, describe how to use them.)

- Visualizing bounces separately
	- Bounces can be visualized separately by pressing "Show only certain bounce (B)" UI button or pressing B.
	With the right most slider you can choose which bounce (between -1 and 8, where -1 is all bounces and 0 is only direct light) will be seen after pressing the button.
	If this button is pressed before computing radiosity ( = before pressing "Compute Radiosity (ENTER)" or "Enter") all vertice colors are
	initialized to be black (only the light source will be visible).

- Quasi Monte Carlo sampling
	- Halton sampling
	- Halton sampling is used for taking sample points from light source and for taking directions to indirect lightning.
	- Halton sampling can be enabled with toggle button "Sampling mode: Halton".

- Stratified sampling
	- Stratified sampling is used for taking sample points from light source and for taking random directions to indirect lightning.
	- Stratified sampling can be enabled with toggle button "Sampling mode: Stratified".
	
	- The number of samples is the integer of the square root of "Direct lightning rays" (can be changed in UI) to the power of 2.
	-> ((int) sqrt(numberOfRays)) ** 2. For example 400 rays is 400 samples, 500 rays is 484 samples (sqrt(500) = 22.36 -> 22 ^ 2 = 484), etc.


# Are there any known problems/bugs remaining in your code?

(Please provide a list of the problems. If possible, describe what you think the cause is, how you have attempted to diagnose or fix the problem, and how you would attempt to diagnose or fix it if you had more time or motivation. This is important: we are more likely to assign partial credit if you help us understand what's going on.)

# Did you collaborate with anyone in the class?

(Did you help others? Did others help you? Let us know who you talked to, and what sort of help you gave or received.)
Collaborated with Satria Sutisna, helped each other with bugs

# Any other comments you'd like to share about the assignment or the course so far?

(Was the assignment too long? Too hard? Fun or boring? Did you learn something, or was it a total waste of time? Can we do something differently to help you learn? Please be brutally honest; we won't take it personally.)

