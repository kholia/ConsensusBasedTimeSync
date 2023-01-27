### Consensus Based (Relative) Time Synchronization

We use Si4732 to receiving radio signals. The audio output of Si4732 is fed to
the ADC of Raspberry Pi Pico.


#### Hardware Requirements

- Raspberry Pi Pico (or a compatible) board

- Si4732-BoB (breakout board)

- Bunch of jumper wires

- Si5351 module (provides clock to Si4732-BoB)


#### Connections

Si5351 module:

- SDA to GPIO12
- SCL to GPIO13
- ADC on GPIO26

Si4732 BoB connections:

- SDA (12) to GPIO12
- SCL (11) to GPIO13
- Pin 1: Audio Output
- Pin 8: Antenna
- Pin 7 and Pin 15: Ground
- Pin 14: 3.3v ONLY!
- Pin 13: Clock input


#### Build Steps

OS used: Xubuntu 22.04.1 LTS

Follow https://github.com/raspberrypi/pico-sdk#quick-start-your-own-project

```
mkdir -p ~/repos

cd ~/repos

git clone https://github.com/raspberrypi/pico-sdk.git

export PICO_SDK_PATH=${HOME}/repos/pico-sdk

git clone https://github.com/kholia/ConsensusBasedTimeSync.git

cd ConsensusBasedTimeSync

cmake .

make
```

Done. At the end of this process a `run_ft8.uf2` file is generated which can be
uploaded to the Pico (also W) board.


#### Usage

Monitor the serial output from the Pico (W) board:

```
$ reset; pyserial-miniterm /dev/ttyACM0 115200
```
