

void ontwijkobject (void)
{

 //naar rechts 
 //    |  
 //    .-.
 //    X |
 //    R-.
 //    |
 OnFwd(motorLeft, 60);
 OnFwd(motorRight, -60);
 Wait(300);
 
 //rechtdoor
 //    |  
 //    .-.
 //    X |
 //    .R.
 //    |
 OnFwd(motorLeft, 100);
 OnFwd(motorRight, 100);
 Wait(500);
 
 //naar links
 //    |  
 //    .-.
 //    X |
 //    .-R
 //    |
 OnFwd(motorLeft, -60);
 OnFwd(motorRight, 60);
 Wait(300);
 
 //rechtdoor
 //    |  
 //    .-.
 //    X R
 //    .-.
 //    |
 OnFwd(motorLeft, 100);
 OnFwd(motorRight, 100);
 Wait(1000);
 
 //naar links
 //    |  
 //    .-R
 //    X |
 //    .-.
 //    |
 OnFwd(motorLeft, -60);
 OnFwd(motorRight, 60);
 Wait(300);
 
 //rechtdoor
 //    |  
 //    .R.
 //    X |
 //    .-.
 //    |
 
 OnFwd(motorLeft, 100);
 OnFwd(motorRight, 100);
 Wait(500);
 
 //naar rechts
 //    |  
 //    R-.
 //    X |
 //    .-.
 //    |
 
 OnFwd(motorLeft, 60);
 OnFwd(motorRight, -60);
 Wait(300);
 


}