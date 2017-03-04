#include "stdafx.h"
#include "DiffieHellman.h"
// CRYPTO LIBRARY FOR EXCHANGING KEYS
// USING THE DIFFIE-HELLMAN KEY EXCHANGE PROTOCOL

// The diffie-hellman can be used to securely exchange keys
// between parties, where a third party eavesdropper given
// the values being transmitted cannot determine the key.

// Implemented by Lee Griffiths, Jan 2004.
// This software is freeware, you may use it to your discretion,
// however by doing so you take full responsibility for any damage
// it may cause.

// Hope you find it useful, even if you just use some of the functions
// out of it like the prime number generator and the XtoYmodN function.

// It would be great if you could send me emails to: griffter@hotmail.co.uk
// with any suggestions, comments, or questions!
#define DH_G_NUMBER 13

#ifdef TEACHER_VERSION

CDiffieHellman::CDiffieHellman(void)
{

}

CDiffieHellman::~CDiffieHellman(void)
{

} 
//Raises X to the power Y in modulus N. the values of X, Y, and N can be massive, 
//and this can be acheived by first calculating X to the power of 2 then 
//using power chaining over modulus N
__int64 CDiffieHellman::XpowYmodN(__int64 x, __int64 y, __int64 N)
{
	__int64 tmp = 0;
	if (y==1) return (x % N);

	if ((y&1)==0)
	{
		tmp = XpowYmodN(x,y/2,N);
		return ((tmp * tmp) % N);
	}
	else
	{
		tmp = XpowYmodN(x,(y-1)/2,N);
		tmp = ((tmp * tmp) % N);
		tmp = ((tmp * x) % N);
		return (tmp);
	}
}


//Performs the miller-rabin primality test on a guessed prime n.
//trials is the number of attempts to verify this, because the function
//is not 100% accurate it may be a composite.  However setting the trial
//value to around 5 should guarantee success even with very large primes
bool CDiffieHellman::MillerRabin (__int64 n) 
{ 
	__int64 a = 0; 

	for (int i=0; i<5; i++)  //5 trials to make a reasonable test
	{ 
		a = (rand() % (n-3))+2;// gets random value in [2..n-1] 
		
		//test if n is prime
		__int64 d = XpowYmodN(a, n-1, n); 
		if (d!=1) return false;
	} 
	
	return true; // n probably prime 
} 



// Generates a random number by first getting the RTSC of the CPU, then 
// uses a Linear feedback shift register.
// The RTSC is then added to fill the 64-bits
unsigned __int64 CDiffieHellman::GenerateRandomNumber(void)
{
  static unsigned long dh_rnd = 0x41594c49;
  static unsigned long dh_x   = 0x94c49514;

  LFSR(dh_x); 
  dh_rnd^=((unsigned long)GetTickCount())^dh_x; 
  ROT(dh_rnd,7);

  return (unsigned __int64)GetTickCount() + dh_rnd;
} 



//Generates a large prime number by
//choosing a randomly large integer, and ensuring the value is odd
//then uses the miller-rabin primality test on it to see if it is prime
//if not the value gets increased until it is prime
unsigned __int64 CDiffieHellman::GeneratePrime()
{
	unsigned __int64 tmp = 0;

	tmp	=  GenerateRandomNumber() % MAX_PRIME_NUMBER;

	//ensure it is an odd number
	if ((tmp & 1)==0) tmp+=1;

	if (MillerRabin(tmp)==true) return tmp;
	
	do
	{
		tmp+=2;	
	} while (MillerRabin(tmp)==false);
		
	return tmp;
}


//generates two important numbers  'N' and 'a'(the last one is derived from password and is 
//kept secret). It then calculates number 'X'. Numbers 'N' and 'X' make public key 
//and are transferred to the other side to be used for encription.
void CDiffieHellman::DerivePublicKey(char *password, __int64 *N, __int64 *X)
{
	*N = GeneratePrime();
	while (*N<DH_G_NUMBER) *N=GeneratePrime();

	char pswd[32];
	memset(pswd,0,32);
	for (int i=0;i<(int)strlen(password);i++)
		pswd[i%8]+=password[i];

	a=*((__int64*)pswd); 
	a=a%MAX_RANDOM_INTEGER;
	if (a==0) a=0x112233;

	if (X!=NULL) *X = XpowYmodN(DH_G_NUMBER,a,*N);
}

//creates decription key from helper value 'Y', 'N' (part of public key) and 'a' (secret part)
//The decription key is equal to encription key but can be only calculated if 'a' is known.
void CDiffieHellman::CreateDecryptionKey(__int64 Y, __int64 N, __int64 *Key)
{
	*Key = XpowYmodN(Y,a,N);
}

//creates encription key form given public key 'G', 'N' and 'X'. The encription key '*Key'
//is kept secret, but the helper value 'Y' is to be transferred to the other side to assist
//during decription. (by knowing only numbers 'G', 'N', 'X' and 'Y' it is not possible to
//make decription - the number 'a' is needed and is only known on the other side.)
void CDiffieHellman::CreateEncryptionKey(__int64 N, __int64 X,__int64 *Key, __int64 *Y)
{
	__int64 b = (__int64) (GenerateRandomNumber() % MAX_RANDOM_INTEGER);
	*Y = XpowYmodN(DH_G_NUMBER,b,N);
	*Key = XpowYmodN(X,b,N);
}

#endif