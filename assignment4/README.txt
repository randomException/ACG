﻿# ME-E4100 Advanced Computer Graphics, Spring 2017
# Lehtinen / Härkönen, Ollikainen
#
# Assignment 4: Path Tracing

Student name: Valtteri Wahlroos
Student number: 357096

# Which parts of the assignment did you complete? Mark them 'done'.
# You can also mark non-completed parts as 'attempted' if you spent a fair amount of
# effort on them. If you do, explain the work you did in the problems/bugs section
# and leave your 'attempt' code in place (commented out if necessary) so we can see it.

R1 Integrate your raytracer (1p): done
  R2 Direct light & shadows (2p): done
  R3 Bounced indirect light (5p): done
        R4 Russian roulette (2p): done

Remember the other mandatory deliverables!
- 6 images rendered with our default presets
- Rendering competition PDF + images in separate folder

# Did you do any extra credit work?

(Describe what you did and, if there was a substantial amount of work involved, how you did it. If your extra features are interactive or can be toggled, describe how to use them.)

- Implement some informative visualizer for your path tracer.
	- Shoot visualized tracer paths by pressing Ctrl. There is following visualizations in addition for already given ones:
		- Shadow ray as yellow.
		- Trace paths which are shoot by russian roulette as blue.

- Implement a system that supports light-emitting triangles in the scene.
	- Light-emitting triangles only in Cornell scene.
	- Enable/Disable with a button at the bottom of UI "Extra: Use Emission Triangles".
	- Light triangles are sampled according to their emissive power.

# Have you done extra credit work on previous rounds whose grading we have postponed to this round?

(Are all the features integrated into your Assn4 submission? If not, point out which one of your submissions we should use to grade those features.)

# Are there any known problems/bugs remaining in your code?

(Please provide a list of the problems. If possible, describe what you think the cause is, how you have attempted to diagnose or fix the problem, and how you would attempt to diagnose or fix it if you had more time or motivation. This is important: we are more likely to assign partial credit if you help us understand what's going on.)

# Did you collaborate with anyone in the class?

(Did you help others? Did others help you? Let us know who you talked to, and what sort of help you gave or received.)
Collaborated with Satria Sutisna, helped each other with bugs.

# Any other comments you'd like to share about the assignment or the course overall?

(Was the assignment too long? Too hard? Fun or boring? Did you learn something, or was it a total waste of time? Can we do something differently to help you learn? Please be brutally honest; we won't take it personally.)

