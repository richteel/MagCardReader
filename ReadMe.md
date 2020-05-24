# MagCardReader

<p>
Platform: Arduino<br />  
References:
</p>

- [magstripelib](https://github.com/carlosefr/magstripelib)
- [Magnetic Card Reader (TTL) with Arduino](https://teelsys.com/?p=708) 
- [Magnetic Card Reader (TTL) with Arduino Part II](https://teelsys.com/?p=767)  
  
<p>
<img src="/images/20200523_115539_c.jpg" alt="Board with Arduino Nano and two magnetic card readers" width="400px" />
</p>

Code to allow an Arduino to read from more than one magnetic card reader. The code was modified from the [magstripelib](https://github.com/carlosefr/magstripelib) Arduino library. If you only need to read from one card reader and the data line of your reader is free from noise then you should consider using the [magstripelib](https://github.com/carlosefr/magstripelib) library instead of the code presented here.

In addition to the code, there are some photos and Inkscape files for laser cutting a board to mount two Tokin MCS-131P-3 magnetic card readers and an [Adafruit Perma-Proto Half-sized Breadboard PCB](https://www.adafruit.com/product/1609).

When selecting the data pin, keep in mind which pins may be used as interrupts. The Uno, Nano, Mini, & other 328-based boards have only two pins available with interrupts. Other boards have more but many have limited pins available. Check the Arduino reference page at [https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/](https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/) for details.

For more information regarding this project, please visit the two part blog post listing in the references above.