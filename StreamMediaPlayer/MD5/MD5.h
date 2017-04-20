#ifndef MD5_H  
#define MD5_H  

typedef struct  
{  
	unsigned int count[2];  
	unsigned int state[4];  
	unsigned char buffer[64];     
}MD5_CTX;  


#define HASHLEN 16
typedef char HASH[HASHLEN];

#define HASHHEXLEN 32
typedef char HASHHEX[HASHHEXLEN + 1];


#define F(x,y,z) ((x & y) | (~x & z))  
#define G(x,y,z) ((x & z) | (y & ~z))  
#define H(x,y,z) (x^y^z)  
#define I(x,y,z) (y ^ (x | ~z))  
#define ROTATE_LEFT(x,n) ((x << n) | (x >> (32-n)))
#define FF(a,b,c,d,x,s,ac)\
{\
	a += F(b,c,d) + x + ac;\
	a = ROTATE_LEFT(a,s);\
	a += b;\
}
#define GG(a,b,c,d,x,s,ac)\
{\
	a += G(b,c,d) + x + ac;\
	a = ROTATE_LEFT(a,s);\
	a += b;\
}
#define HH(a,b,c,d,x,s,ac)\
{\
	a += H(b,c,d) + x + ac;\
	a = ROTATE_LEFT(a,s);\
	a += b;\
}
#define II(a,b,c,d,x,s,ac)\
{\
	a += I(b,c,d) + x + ac;\
	a = ROTATE_LEFT(a,s);\
	a += b;\
}
void MD5Init(MD5_CTX *context);  
void MD5Update(MD5_CTX *context,unsigned char *input,unsigned int inputlen);  
void MD5Final(unsigned char digest[16], MD5_CTX *context);  
void MD5Transform(unsigned int state[4],unsigned char block[64]);  
void MD5Encode(unsigned char *output,unsigned int *input,unsigned int len);  
void MD5Decode(unsigned int *output,unsigned char *input,unsigned int len);  
void NewDigestCalcHA1(char * pszUserName, char * pszRealm, char * pszPassword, HASHHEX SessionKey);
void CvtHex(HASH Bin, HASHHEX Hex);
void getNewDigestResponse(char *pszNonce, char *pszUser, char *pszRealm, char *pszPass, char *pszMethod, char *pszURI, HASHHEX Response);
void NewDigestCalcResponse( 
						   HASHHEX HA1,           /* H(A1) */ 
						   char * pszNonce,       /* nonce from server */ 
						   char * pszDigestUri,   /* requested URL */ 
						   char * pszMethod,      /* requested method */ 
						   HASHHEX Response      /* request-digest or response-digest */ 
						   );
#endif  