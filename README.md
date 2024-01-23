Ceci n'est qu'une ébauche pour vérifier si c'est possible d'utiliser le code c++ d'un arduino et de le transférer directement sur un Rapsberry Pi en c++.

N.B. il faut installer la librarie gpiod
sudo apt-get install gpiod libgpiod-dev libgpiod-doc

(présentement c'est seulement les GPIOs qui fonctionnent).
Il reste à insérer le code pour le LCD et les capteurs de température.

tout simplement  faire<br>
make<br>
./test

