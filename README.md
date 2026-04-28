# CMP407 Assessment Project - Poor, Pitted and Powerful

## **Eryth Davidson - 2201593**

### A 2d platformer/PvE arena shooter. (Work in progress!)

This project is based on my CMP425 network programming project. You can see what this project initially started with by checking out that repo; https://github.com/Abertay-University-SDI/assessment-project-ErythiaSea


Controls
---
- A/D - move left/right
- Space - jump
- LMB - fire weapon
- Enter - start match (host only)
- G - spawn enemy (debug)
- P - print debug information to console


Instructions
---
When opening, if port 54940 (defined as hostPort in NetworkManager.h) is available, you will automatically host a lobby. If not, you can attempt to host on another port, or attempt to join a lobby at any port. The host can start a match when another player joins them.

The objective is to collect more coins than your opponents before time runs out! Slain enemies will drop coins - but if you're hit, so will you! You score a point if you have more than your opponents. Win by getting enough points!


Credits
---
Uses a framework developed by me and Ari; https://github.com/EssithAbertay/Framework

Assets from:

- Cave Story (Studio Pixel, 2004)
- Terraria (Relogic, 2011)
- Dr. Robotnik's Ring Racers (Kart Krew Dev, 2024)
