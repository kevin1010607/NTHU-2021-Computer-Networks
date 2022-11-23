/****************************notice******************************
 * 
 * It's a example for test how many time your function execute.
 * You can check the Ack timeout like this example.
 * Usage: gcc test_clock.c -o test_clock -lm
 *        ./test_clock
 * 
 ***************************************************************/
#include <stdio.h>      
#include <time.h>       
#include <math.h>       

//Some busy function
int frequency_of_primes (int n) {
  int i,j;
  int freq=n-1;
  for (i=2; i<=n; ++i) for (j=sqrt(i);j>1;--j) if (i%j==0) {--freq; break;}
  return freq;
}

int main ()
{
  clock_t t;
  int f;
  t = (clock()*1000)/CLOCKS_PER_SEC;
  printf ("Calculating...\n");
  f = frequency_of_primes (999999);
  printf ("The number of primes lower than 100,000 is: %d\n",f);
  t = (clock()*1000)/CLOCKS_PER_SEC - t;
  printf ("It took me %ld milliseconds.\n",t);
  return 0;
}
