# ASO_P1_LKM
Practica 1 d'Administració de Sistemes Operatius, on es programa un LKM (Loadable Kernel Module) sobre una RaspberryPi que fa servir els GPIOs de la placa per a interactuar amb LEDs i polsadors. Repositori de git a https://github.com/rogergalvan/ASO_P1_LKM

El respositori consta de dos fitxers principals, el codi **fase1.c** i el **Makefile**. El primer conté el codi font per al nostre LKM, mentre que el Makefile permet compilar el codi, instalar el LKM així com netejar. Aquesta pràctica està pensada per a executar-se sobre una RaspberryPi amb Raspberry Pi OS, i que els linux headers adients estan apropiadament instalats. Sino sempre podem fer un: `sudo apt-get install raspberrypi-kernel-headers`
Per a fer servir el LKM, haurem de fer un `make` dins el directori on tinguem aquests dos arxius per a compilar el codi. Tot seguit, farem un `sudo make install` per a instalar el fitxer .ko, i ja tindrem el nostre LKM engegat i llest per a fer servir. Per últim, podem fer un `make clean` per a netejar els fitxer brossa generats.
Un cop fet tot això ja podrem testejar el LKM amb els LEDs i els polsadors connectats als ports GPIO escpecificats al principi del codi .c
