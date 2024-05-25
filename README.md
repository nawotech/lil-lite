# Lil' Lite
The World's Funnest Bike Light!

https://nawo.tech/lil-lite/

This repo contains the firmware running on the light. For the PCB design: https://github.com/nawotech/lil-lite-board

## Features

### Selectable Pattern

Choose one of the built in patterns with the button, or create your own and upload using Arduino and the USB-C port.

![patterns-1](https://github.com/nawotech/lil-lite/assets/85331937/29c69c70-7d7b-4aeb-bab9-8ae9f6ae597b)

### Never worry about turning on or off your light again

Lil Lite has a built in light sensor and accelerometer to detect

when you start riding at night (light ON)
when you park your bike (light OFF)

![riding-1](https://github.com/nawotech/lil-lite/assets/85331937/657fdb17-a6f1-4f3d-ad17-641845974434)

### Easy battery level checking

Never get stuck with a dead light, any time you can see exactly how much battery your light has, including when it turns on

![battery](https://github.com/nawotech/lil-lite/assets/85331937/2d37ac40-ad31-405e-a289-e03c91f4607a)


### Charge with USB-C

The future is now, it's time your bike light got with the times :)

![charging](https://github.com/nawotech/lil-lite/assets/85331937/5e90d103-e322-46d9-9860-0727fc346554)

### Easy to mount

The case fits against the range of bike seat posts, and 2 bands sizes are included to fit almost any bike

![mounting-2](https://github.com/nawotech/lil-lite/assets/85331937/0bdb2481-4c56-45c7-818a-b7e8c6105fe8)


## How to use
### Mounting to bike
- 2 bands are included, select the one sized best for your bike seat post
- Clip the band to one side of the light
- Place on the back of the seat post, and wrap the band around, clip the other side of the band to the other side of the light


### Turning on
- Press the button to turn on
- The display will show the current battery level, then the pattern after several seconds
- The light will now turn itself on and off automatically
- After your bike is not moving for ~ 30 sec, the light will power down
- The next time you go to move the bike and it's dark out, the light will come on by itself

### Charging
- When the battery gets low, take the light off your bike seat post
- Connect a USB-C charging cable to the light
- The display will show the progress of the charge, it takes ~ 4 hr to fully charge
- The display will show all lights green fading on and off slowly when it is done

### Checking battery level
- Press the button when the light is running to view battery level

### Changing the pattern
- Press the button when the light is running, the battery level will show
- Press again within 2 sec to cycle through patterns, keep pressing to view the next pattern
- Once you see the pattern you want, wait (don't press button) and after 3 sec the new pattern will be set

### Manually turning off
- If you need to keep the light from turning on by itself, you can manually turn it off
- First, make sure the light is on and showing the pattern
- Then, press and hold the button and wait until the lights turn fully off, then release the button
- It is now off, see "turning on" on how to turn it back on again manually

## Standard Patterns
- [X] Blink red
- [X] Bink pink
- [X] Bouncing ball
- [X] Grow flash red
- [ ] Red and random color flash

## How to program Lil' Lite
1. Download and install Viusal Studio Code (VSCode)
2. Open VSCode, click the "extensions" icon on the left bar, looks like 4 squares with 1 square leaving
3. Search "platformIO" and install the PlatformIO extension
4. Clone the Lil Lite repo to your computer (see next steps)
5. Create a folder for git called "git" on your hard drive C:/ or home (if you haven't already)
6. Open a terminal window and cd C:/git or wherever you made the git folder
7. Enter `git clone https://github.com/nawotech/lil-lite.git`
8. Open VSCode, and open the "lil lite" folder
9. Plug in Lil Lite to your computer with the USB-C cable
10. Once PlatformIO loads, press the "upload" arrow on the bottom bar to flash the latest firmware
