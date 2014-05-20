#define ONTWIJKZIJDE 900
#define ONTWIJKLENGTE 1600
#define ONTWIJKHOEK 520

unsigned int motorLeft = OUT_B;
unsigned int motorRight = OUT_A;

void ontwijkroutine(void)
{

 //naar rechts
 //    |
 //    .-.
 //    X |
 //    R-.
 //    |
 OnFwd(motorLeft, 100);
 OnFwd(motorRight, 0);
 Wait(ONTWIJKHOEK);

 //rechtdoor
 //    |
 //    .-.
 //    X |
 //    .R.
 //    |
 OnFwd(motorLeft, 100);
 OnFwd(motorRight, 100);
 Wait(ONTWIJKZIJDE);

 //naar links
 //    |
 //    .-.
 //    X |
 //    .-R
 //    |
 OnFwd(motorLeft, 0);
 OnFwd(motorRight, 100);
 Wait(ONTWIJKHOEK);

 //rechtdoor
 //    |
 //    .-.
 //    X R
 //    .-.
 //    |
 OnFwd(motorLeft, 100);
 OnFwd(motorRight, 100);
 Wait(ONTWIJKLENGTE);

 //naar links
 //    |
 //    .-R
 //    X |
 //    .-.
 //    |
 OnFwd(motorLeft, 0);
 OnFwd(motorRight, 100);
 Wait(ONTWIJKHOEK);

 //rechtdoor
 //    |
 //    .R.
 //    X |
 //    .-.
 //    |

 OnFwd(motorLeft, 100);
 OnFwd(motorRight, 100);
 Wait(ONTWIJKZIJDE);

 //naar rechts
 //    |
 //    R-.
 //    X |
 //    .-.
 //    |

 OnFwd(motorLeft, 100);
 OnFwd(motorRight, 0);
 Wait(ONTWIJKHOEK);
 
 //rechtdoor
 //    R
 //    .-.
 //    X |
 //    .-.
 //    |

 OnFwd(motorLeft, 100);
 OnFwd(motorRight, 100);
 Wait(1000);



}

task main()
{

	ontwijkroutine();

}