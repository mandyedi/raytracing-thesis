# lovecraft_city.sc
#
# Claude prompt (Opus 5.5, Extra High):
# Create me another scene. Be insipred by H.P. Lovecraft novels:
# - At the Mountains of Madness
# - The Shadow Out of Time
# - and some other novels that have a similar plot
#
# Put the viewer inside the huge underground city. Create an atmosphere that can represent Lovecraft's world.
#
# The viewer stands on a high gallery in the cavern wall, lantern in hand, looking over a buried
# city of the Old Ones and the Great Race. In the plaza below, a square abyss plunges into the
# earth, and a sickly green light wells up out of it: it lights the towers and bridges around it
# from below and throws the shape of the shaft onto the vault. On a terrace beside the abyss the
# cone-shaped Great Race of Yith gather around a sealed trapdoor, in pale light falling through a
# rift in the vault. A shoggoth spills over the far rim of the shaft. Far behind the city a violet
# glow outlines more towers and the pillars that hold up the vault.
#
# Stalactites are cones with a negative y scale (hung upside down).
# Units are metres; the Great Race are about 4 m tall.

# Camera: on the gallery, 2 m above its floor
c 0 30 72 0 18.5963 0

# The cavern: plaza floor with the abyss cut into it, the walls, and the vault with a rift in it
o 2 0 -80 31.25 440 80 72.5 0.22 0.22 0.23 0 1 0 2 0 0
o 2 0 -80 -102.5 440 80 115 0.22 0.22 0.23 0 1 0 2 0 0
o 2 -118.5 -80 -25 203 80 40 0.22 0.22 0.23 0 1 0 2 0 0
o 2 118.5 -80 -25 203 80 40 0.22 0.22 0.23 0 1 0 2 0 0
o 2 0 -82 -25 34 2 40 0.22 0.22 0.23 0 1 0 2 0 0
o 2 0 -160 -155 440 230 10 0.22 0.22 0.23 0 1 0 2 0 0
o 2 -210 -160 -25 20 230 270 0.22 0.22 0.23 0 1 0 2 0 0
o 2 210 -160 -25 20 230 270 0.22 0.22 0.23 0 1 0 2 0 0
o 2 0 -160 105 440 230 10 0.22 0.22 0.23 0 1 0 2 0 0
o 2 0 52 -82.8864 440 4 154.2273 0.12 0.12 0.14 0 1 0 2 0 0
o 2 0 52 60.1136 440 4 99.7727 0.12 0.12 0.14 0 1 0 2 0 0
o 2 -90.9886 52 2.2273 258.0227 4 16 0.12 0.12 0.14 0 1 0 2 0 0
o 2 137.0114 52 2.2273 165.9773 4 16 0.12 0.12 0.14 0 1 0 2 0 0

# Remnants of the spiral ramp that winds down the walls of the abyss
o 2 4 -5.5 -43.75 26 1 2.5 0.22 0.22 0.23 0 1 0 2 0 0
o 2 -15.75 -9 -33 2.5 1 18 0.22 0.22 0.23 0 1 0 2 0 0

# The gallery the viewer stands on, a colossal pier beside it, and a metal book case left
# on the flagstones (in The Shadow Out of Time, Peaslee finds his own handwriting in one)
o 2 0 -80 83.75 440 108 32.5 0.52 0.47 0.41 0 1 0 2 0 0
o 2 -9.25 0 60 5.5 53 8 0.52 0.47 0.41 0 1 0 2 0 0
o 2 -1.65 28 68.75 3.1 0.12 2.5 0.52 0.47 0.41 0 1 0 2 0 0
o 2 1.725 28 68.75 3.35 0.05 2.5 0.52 0.47 0.41 0 1 0 2 0 0
o 2 -0.8 28.12 67.93 0.5 0.1 0.36 0.5 0.4 0.24 2 0.9 0.7 24 0 0

# Lantern shade, behind the camera: it keeps the lantern light on the gallery floor
o 2 0 30.56 72.95 8 3.44 0.1 0.07 0.07 0.09 0 1 0 2 0 0

# Stepped towers of the Old Ones on the left rim of the abyss and behind it
o 2 -23 0 -27 8 34 26 0.3 0.35 0.32 0 1 0 2 0 0
o 2 -23 34 -27 5 5 12 0.3 0.35 0.32 0 1 0 2 0 0
o 3 -23 39 -27 2.5 5 2.5 0.3 0.35 0.32 0 1 0 2 0 0
o 2 -34 0 -64 20 42 16 0.3 0.35 0.32 0 1 0 2 0 0
o 2 -34 42 -64 14 4 12 0.3 0.35 0.32 0 1 0 2 0 0

# Windowless basalt tower on the far right corner, a bridge high across the abyss
# and a causeway across its mouth
o 2 23 0 -40 8 34 8 0.07 0.07 0.09 0 1 0 2 0 0
o 2 0 17 -38.5 38 1.8 3 0.16 0.17 0.19 0 1 0 2 0 0
o 2 0 -1.5 -12 34 1.5 4 0.16 0.17 0.19 0 1 0 2 0 0

# A black obelisk at the near rim
o 2 -22.5 0 -2.5 3 24 3 0.07 0.07 0.09 2 0.9 0.3 30 0 0
o 3 -22.5 24 -2.5 1.5 3 1.5 0.07 0.07 0.09 2 0.9 0.3 30 0 0

# The temple: a stepped cone behind the abyss, the shape of the Great Race in stone
o 4 0.4279 -0.0546 -68.7893 16 6 16 0.3 0.35 0.32 0 1 0 2 0 1
o 4 0.3476 11.9545 -68.6413 13 5 13 0.3 0.35 0.32 0 1 0 2 0 1
o 4 0.2808 21.9591 -68.518 10.5 4.5 10.5 0.3 0.35 0.32 0 1 0 2 0 1
o 4 0.2139 30.9636 -68.3946 8 4 8 0.3 0.35 0.32 0 1 0 2 0 1
o 6 0.3042 21.5302 -79.4153 10.88 1.8 10.88 0.4 0.42 0.4 0 1 0 2 0 1
o 6 0.2483 30.5824 -77.3169 8.88 1.6 8.88 0.4 0.42 0.4 0 1 0 2 0 1
o 5 0.1471 38.9454 -68.2713 5.5 6 5.5 0.3 0.35 0.32 0 1 0 2 0 1

# The great cylindrical tower on the right, and the bridge from it to the temple
o 4 32.2942 -0.182 -58.5426 11 20 11 0.16 0.17 0.19 0 1 0 2 0 1
o 6 32.2595 12.3736 -67.7366 9.28 2.4 9.28 0.4 0.42 0.4 0 1 0 2 0 1
o 6 32.2595 26.3736 -67.7366 9.28 2.4 9.28 0.4 0.42 0.4 0 1 0 2 0 1
o 6 32.2774 38.956 -68.4081 9.92 4 9.92 0.16 0.17 0.19 0 1 0 2 0 1
o 2 14.5 25 -61 17 2 4 0.4 0.42 0.4 0 1 0 2 0 0

# The shoggoth: a glossy black mass of bubbles and eyes spilling over the far rim
o 1 -4 -2.1106 -46 5.2 5.2 5.2 0.02 0.03 0.03 2 1 1 48 0 1
o 1 -1 -1.009 -47 4.4 4.4 4.4 0.02 0.03 0.03 2 1 1 48 0 1
o 1 -7 -1.2082 -47.5 4 4 4 0.02 0.03 0.03 2 1 1 48 0 1
o 1 -2.5 1.3927 -47.5 3.6 3.6 3.6 0.02 0.03 0.03 2 1 1 48 0 1
o 1 -5.5 1.3935 -48 3.2 3.2 3.2 0.02 0.03 0.03 2 1 1 48 0 1
o 1 0.8 -2.2069 -45.5 3.4 3.4 3.4 0.02 0.03 0.03 2 1 1 48 0 1
o 1 -8.5 -2.7061 -45.2 3 3 3 0.02 0.03 0.03 2 1 1 48 0 1
o 1 -3.5 -4.0065 -44.3 3.2 3.2 3.2 0.02 0.03 0.03 2 1 1 48 0 1
o 1 -4.6 0.9984 -43.6 0.8 0.8 0.8 0.85 0.95 0.55 2 1 0.8 16 0 1
o 1 -3 -0.1012 -43.5 0.6 0.6 0.6 0.85 0.95 0.55 2 1 0.8 16 0 1
o 1 -1.6 1.5787 -44.9 0.64 0.64 0.64 0.85 0.95 0.55 2 1 0.8 16 0 1
o 1 -7.2 0.9189 -45.6 0.56 0.56 0.56 0.85 0.95 0.55 2 1 0.8 16 0 1
o 1 -2.9 3.349 -45.8 0.5 0.5 0.5 0.85 0.95 0.55 2 1 0.8 16 0 1
o 1 0.6 -0.3611 -43.9 0.52 0.52 0.52 0.85 0.95 0.55 2 1 0.8 16 0 1

# Terrace beside the abyss: five of the Great Race around a sealed trapdoor
o 2 30 0 -17 22 5 18 0.4 0.42 0.4 0 1 0 2 0 0
o 4 30.1123 4.9986 -17.2072 4.2 0.15 4.2 0.5 0.4 0.24 2 0.9 0.5 20 0 1
o 6 30.1029 4.989 -20.8611 3.68 1 3.68 0.16 0.17 0.19 0 1 0 2 0 1
o 5 23.5321 4.9818 -16.0592 1.2 2 1.2 0.56 0.58 0.5 2 0.9 0.5 20 0 1
o 4 23.5032 8.5964 -16.0059 0.12 0.4 0.12 0.56 0.58 0.5 0 1 0 2 0 1
o 1 23.5 9.2984 -16 0.8 0.8 0.8 0.8 0.72 0.42 2 1 0.4 16 0 1
o 5 26.5321 4.9818 -22.5592 1.2 2 1.2 0.56 0.58 0.5 2 0.9 0.5 20 0 1
o 4 26.5032 8.5964 -22.5059 0.12 0.4 0.12 0.56 0.58 0.5 0 1 0 2 0 1
o 1 26.5 9.2984 -22.5 0.8 0.8 0.8 0.8 0.72 0.42 2 1 0.4 16 0 1
o 5 33.0321 4.9818 -23.0592 1.2 2 1.2 0.56 0.58 0.5 2 0.9 0.5 20 0 1
o 4 33.0032 8.5964 -23.0059 0.12 0.4 0.12 0.56 0.58 0.5 0 1 0 2 0 1
o 1 33 9.2984 -23 0.8 0.8 0.8 0.8 0.72 0.42 2 1 0.4 16 0 1
o 5 36.5321 4.9818 -16.5592 1.2 2 1.2 0.56 0.58 0.5 2 0.9 0.5 20 0 1
o 4 36.5032 8.5964 -16.5059 0.12 0.4 0.12 0.56 0.58 0.5 0 1 0 2 0 1
o 1 36.5 9.2984 -16.5 0.8 0.8 0.8 0.8 0.72 0.42 2 1 0.4 16 0 1
o 5 31.5321 4.9818 -11.5592 1.2 2 1.2 0.56 0.58 0.5 2 0.9 0.5 20 0 1
o 4 31.5032 8.5964 -11.5059 0.12 0.4 0.12 0.56 0.58 0.5 0 1 0 2 0 1
o 1 31.5 9.2984 -11.5 0.8 0.8 0.8 0.8 0.72 0.42 2 1 0.4 16 0 1

# Terraced ziggurat of the Old Ones, left, in darkness
o 2 -52 0 -68 24 7 24 0.16 0.17 0.19 0 1 0 2 0 0
o 2 -52 7 -68 18 7 18 0.16 0.17 0.19 0 1 0 2 0 0
o 2 -52 14 -68 12 7 12 0.16 0.17 0.19 0 1 0 2 0 0
o 2 -52 21 -68 6 7 6 0.16 0.17 0.19 0 1 0 2 0 0

# Pillars that hold up the vault
o 4 -44.8663 -0.2456 -100.2467 5 27 5 0.16 0.17 0.19 0 1 0 2 0 1
o 4 58.1605 -0.2456 -92.296 6 27 6 0.16 0.17 0.19 0 1 0 2 0 1
o 4 -19.8797 -0.2456 -128.222 4.5 27 4.5 0.16 0.17 0.19 0 1 0 2 0 1
o 4 35.1337 -0.2456 -135.2467 5 27 5 0.16 0.17 0.19 0 1 0 2 0 1

# Towers in the far distance, outlined by the violet glow
o 2 -76.5 0 -110 17 40 20 0.16 0.17 0.19 0 1 0 2 0 0
o 3 -76.5 40 -110 8.5 10 8.5 0.16 0.17 0.19 0 1 0 2 0 0
o 2 78 0 -115 16 46 20 0.16 0.17 0.19 0 1 0 2 0 0
o 2 54 0 -136 12 30 12 0.3 0.35 0.32 0 1 0 2 0 0
o 4 -37.8395 -0.1638 -140.296 6 18 6 0.16 0.17 0.19 0 1 0 2 0 1
o 5 -37.8262 35.9454 -140.3207 6.5 6 6.5 0.16 0.17 0.19 0 1 0 2 0 1
o 3 -30 0 -116 11 18 11 0.3 0.35 0.32 0 1 0 2 0 0

# Stalactites
o 5 -33.9572 52.5409 -38.0789 1.6 -4.5 1.6 0.22 0.22 0.23 0 1 0 2 0 0
o 5 -13.9465 52.5591 -52.0987 2 -6.5 2 0.22 0.22 0.23 0 1 0 2 0 0
o 5 10.0374 52.5364 -44.0691 1.4 -4 1.4 0.22 0.22 0.23 0 1 0 2 0 0
o 5 22.0588 52.5546 -78.1085 2.2 -6 2.2 0.22 0.22 0.23 0 1 0 2 0 0
o 5 -47.9465 52.55 -82.0987 2 -5.5 2 0.22 0.22 0.23 0 1 0 2 0 0
o 5 42.0348 52.5318 -40.0641 1.3 -3.5 1.3 0.22 0.22 0.23 0 1 0 2 0 0
o 5 4.0642 52.5637 -100.1184 2.4 -7 2.4 0.22 0.22 0.23 0 1 0 2 0 0
o 5 -27.9519 52.5455 -112.0888 1.8 -5 1.8 0.22 0.22 0.23 0 1 0 2 0 0
o 5 56.0588 52.5546 -104.1085 2.2 -6 2.2 0.22 0.22 0.23 0 1 0 2 0 0
o 5 -65.9545 52.5409 -58.0839 1.7 -4.5 1.7 0.22 0.22 0.23 0 1 0 2 0 0

# Lights (point lights don't fade with distance, so walls and occluders shape them)
# Green glow deep in the abyss
l 0 0 -22 -25 0.05 0.05 0.05 0.38 1 0.7 2.2
# Violet glow in the far distance
l 0 8 16 -122 0.05 0.05 0.05 0.55 0.42 1 1.4
# Pale light through the rift in the vault (a distant light: the vault blocks it everywhere else)
l 1 -0.300904 -0.882652 -0.361085 0.05 0.05 0.05 0.78 0.86 1 1.5
# The viewer's lantern, just behind and above the eye
l 0 -0.8 31 74 0.05 0.05 0.05 1 0.62 0.32 1.6
