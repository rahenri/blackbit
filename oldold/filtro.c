#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

int main(void)
{
	char c[256];
	int i;
	for(i=0;i<2;i++)
		scanf("%c",&c[i]);
	c[3]=0;
	while(scanf("%c",&c[2])==1)
	{
		if(strcmp(c,"pos")==0)
		{
			while(scanf("%c",c) && c[0]!='(');
			while(scanf("%c",c) && c[0]!=')')
			{
				if(c[0]==',')
					printf("][");
				else
					printf("%c",c[0]);
				
			}
			scanf("%c%c",&c[0],&c[1]);
			continue;
		}
		else
		{
			printf("%c",c[0]);
		}
		memmove(&c[0],&c[1],2);
	}
	c[2]=0;
	printf(c);
	return 0;
}
