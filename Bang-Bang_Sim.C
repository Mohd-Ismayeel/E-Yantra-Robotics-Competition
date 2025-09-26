#include<stdio.h>

int main()
{
    float ct = 20;
    int at = 20;
    float sp = 50;
    bool heat_on = false;

    int i;

    for(i=0;i<=100;i++)
    {
        if(ct<sp)
        {    
            heat_on = true;
        }
        else
        {    
            heat_on = false;
        }

        if(heat_on)
        {
            ct +=sp/10;
        }
        else
        {    
            ct -=at/10;
        }

        printf("\nTemperature values in %dth minute is %.2f",i,ct);

    }
}