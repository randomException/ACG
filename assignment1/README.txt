# ME-E4100 Advanced Computer Graphics, Spring 2017
# Lehtinen / Härkönen, Ollikainen
#
# Assignment 1: Accelerated Ray Tracing

Student name: Valtteri Wahlroos
Student number: 357096

# Which parts of the assignment did you complete? Mark them 'done'.
# You can also mark non-completed parts as 'attempted' if you spent a fair amount of
# effort on them. If you do, explain the work you did in the problems/bugs section
# and leave your 'attempt' code in place (commented out if necessary) so we can see it.

R1 BVH construction and traversal (5p): done
        R2 BVH saving and loading (1p): done
              R3 Simple texturing (1p): done
             R4 Ambient occlusion (2p): done
         R5 Simple multithreading (1p): done

# Did you do any extra credit work?


- NEW SUBMISSION EXTRAS!!!

- BVH Surface Area Heuristics (old scores: 1.5p)
	- SAH can be enabled/disabled on UI with toggle buttons (buttons at the bottom) by choosing another split mode
	- Performance (64bit):   	       scene - MRays/sec (SAH) - MRays/sec (ObjectMedian) - MRays/sec (SpatialMedian)
		      	       		       cornell     5.30               4.80			 4.20
		               		       conferes	   1.70		      0.26			 0.36
                      	       		       sponza	   1.27               0.16			 0.23

- Efficient SAH Building
	- All scenes can be build much faster (e.g. conference ~10s, sponze test ~20s)






(Describe what you did and, if there was a substantial amount of work involved, how you did it. If your extra features are interactive or can be toggled, describe how to use them.)

- OLD SUBMISSION EXTRAS!

- BVH Surface Area Heuristics (3p)
	- SAH can be enabled/disabled on UI with toggle buttons (buttons at the bottom) by choosing another split mode
	- Performance (64bit):   	       scene - MRays/sec (SAH) - MRays/sec (ObjectMedian) - MRays/sec (SpatialMedian)
		      	       		       cornell     3.20               3.45			 3.54
		               		       conferes	   0.25		      0.16			 0.25
                      	       		       sponza	   0.22               0.15			 0.22

- Optimize your tracer (1-3p)
	- I tried different max amount of triangles which leaves could have (between 2 and 50 triangles).
	Number 10 gave best results. Changing the max amount of triangles from 4 to 10 increased the permormance by 0.1 MRays/sec. 

- Own extras:
- Implemented 'SpatialMedian' and 'None' split modes
	- As I used 'ObjectMedian' for the first assignment, I also did SpatialMedian and None split modes.
	- These can be enabled/disabled on UI with toggle buttons (buttons at the bottom) by choosing another split mode

- Split mode names in the binary files
	- Every binary file has for addition to the given name (Hierarchy-hashcode) also the name of the used split modes
	so all different hierarchies can be stored to every scene.

# Are there any known problems/bugs remaining in your code?

(Please provide a list of the problems. If possible, describe what you think the cause is, how you have attempted to diagnose or fix the problem, and how you would attempt to diagnose or fix it if you had more time or motivation. This is important: we are more likely to assign partial credit if you help us understand what's going on.)

# Did you collaborate with anyone in the class?

(Did you help others? Did others help you? Let us know who you talked to, and what sort of help you gave or received.)
 Collaborated with Satria Sutisna - helped each other
 

# Any other comments you'd like to share about the assignment or the course so far?

(Was the assignment too long? Too hard? Fun or boring? Did you learn something, or was it a total waste of time? Can we do something differently to help you learn? Please be brutally honest; we won't take it personally.)

