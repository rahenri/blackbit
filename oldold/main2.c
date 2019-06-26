#include "main.h"

_board Board;
char turn;
_move list[150];
_peca pecas[32];


int main(void)
{
	char mov[128];
	int oc,ol,dc,dl,r;
	char p[3];
	_move best;
	_pos b;
	SetBoard(Board);
	turn=1;
	while(printf("Jogador de brancas(H ou C):") && scanf(" %c",&p[2])==1 && p[2]!='H' && p[2]!='C')
	{
		printf("H ou C por favor\n");
	}	       
	while(printf("Jogador de pretas:") && scanf(" %c",&p[0]) && p[0]!='H' && p[0]!='C')
	{
		printf("H ou C por favor\n");
	}
	PrintBoard(Board);

	while(1)
	{
		if(p[turn+1]=='H')
		{
			printf("->");
			scanf("%s",mov);				
			if(strcmp(mov,"quit")==0)
				break;
			oc=mov[0]-'a';
			ol=mov[1]-'1';
			dc=mov[2]-'a';
			dl=mov[3]-'1';
			if(vl(ol) || vl(oc) || vl(dc) || vl(dl))
				goto invalido;
			if(!(r=ValidateMove(ol,oc,dl,dc,turn)))
				goto invalido;
		}
		else
		{
			printf("Pensando\n");
			printf("%d\n",Compute(turn,&best,4));
			ol=best.ol;
			oc=best.oc;
			dl=best.dl;
			dc=best.dc;
		}
		b=Board.b[dl][dc];
		Board.b[dl][dc]=Board.b[ol][oc];
		Board.b[ol][oc].type=0;
		if(r==castle)
		{
			if(dc>oc)
			{
				Board.b[dl][dc-1].type=hook;
				Board.b[dl][dc-1].owner=turn;
				Board.b[dl][7].type=clear;
			}
			else
			{
				Board.b[dl][dc+1].type=hook;
				Board.b[dl][dc-1].owner=turn;
				Board.b[dl][0].type=clear;
			}
		}
		if(CheckCheck(turn))
		{
			Board.b[ol][oc]=Board.b[dl][dc];
			Board.b[dl][dc]=b;
			goto invalido;
		}
		turn=-turn;
		if(CheckCheck(turn))
		{
			if(ListMoves(list,turn)==0)
			{
				printf("Mate!\n");
				PrintBoard();
				SetBoard();
				turn =1;
			}
			else
			{
				printf("Cheque!\n");
			}
		}
		PrintBoard();
		continue;
invalido:
		printf("Movimento incorreto\n");
	}
	return 0;
}

void ClearBoard()
{
	memset(Board.b,0,sizeof(Board.b));
	memset(Board.ca,1,sizeof(Board.ca));
}

void SetBoard()
{
	int i;
	ClearBoard();
	Board.b[0][0].type=Board.b[0][7].type=hook;
	Board.b[0][1].type=Board.b[0][6].type=knight;
	Board.b[0][2].type=Board.b[0][5].type=bishop;
	Board.b[0][3].type=queen;
	Board.b[0][4].type=king;
	Board.b[0][0].owner=Board.b[0][7].owner=white;
	Board.b[0][1].owner=Board.b[0][6].owner=white;
	Board.b[0][2].owner=Board.b[0][5].owner=white;
	Board.b[0][3].owner=white;
	Board.b[0][4].owner=white;
	for(i=0;i<8;i++)
	{
		Board.b[1][i].type=pawn;
		Board.b[1][i].owner=white;
	}
	Board.b[7][0].type=Board.b[7][7].type=hook;
	Board.b[7][1].type=Board.b[7][6].type=knight;
	Board.b[7][2].type=Board.b[7][5].type=bishop;
	Board.b[7][3].type=queen;
	Board.b[7][4].type=king;
	Board.b[7][0].owner=Board.b[7][7].owner=black;
	Board.b[7][1].owner=Board.b[7][6].owner=black;
	Board.b[7][2].owner=Board.b[7][5].owner=black;
	Board.b[7][3].owner=black;
	Board.b[7][4].owner=black;
	for(i=0;i<8;i++)
	{
		Board.b[6][i].type=pawn;
		Board.b[6][i].owner=white;
	}
}

#ifdef WINDOWS
void PrintBoard()
{
	int i,l;
	unsigned char linha[128];
	linha[0]=218;
	linha[1]=196;
	linha[2]=196;
	for(i=1;i<8;i++)
	{
		linha[3*i]=194;
		linha[3*i+1]=196;
		linha[3*i+2]=196;
	}
	linha[3*i]=191;
	linha[3*i+1]=0;
	printf("%s\n",linha);
	for(l=7;;l--)
	{
		for(i=0;i<8;i++)
		{
			linha[3*i]=179;
			if(Board.b[l][i].type==clear)
			{
				linha[3*i+1]=linha[3*i+2]=' ';
				continue;
			}
			linha[3*i+1]=tabela[Board.b[l][i]];
			linha[3*i+2]=tabela2[Board.b[l][i]+1];
		}
		linha[3*i]=179;
		linha[3*i+1]=0;
		printf("%s\n",linha);
		linha[0]=195;
		linha[1]=196;
		linha[2]=196;
		if(l==0)
			break;
		for(i=1;i<8;i++)
		{
			linha[3*i]=197;
			linha[3*i+1]=linha[3*i+2]=196;
		}
		linha[3*i]=180;
		linha[3*i+1]=0;
		printf("%s\n",linha);
	}
	linha[0]=192;
	linha[1]=linha[2]=196;
	for(i=1;i<8;i++)
	{
		linha[3*i]=193;
		linha[3*i+1]=linha[3*i+2]=196;
	}
	linha[3*i]=217;
	linha[3*i+1]=0;
	printf("%s\n",linha);
}
#else
void PrintBoard()
{
	int i,l;
	unsigned char linha[128];
	linha[0]='.';
	linha[1]='-';
	linha[2]='-';
	for(i=1;i<8;i++)
	{
		linha[3*i]='-';
		linha[3*i+1]='-';
		linha[3*i+2]='-';
	}
	linha[3*i]='.';
	linha[3*i+1]=0;
	printf("%s\n",linha);
	for(l=7;;l--)
	{
		for(i=0;i<8;i++)
		{
			linha[3*i]='|';
			if(Board.b[l][i].type==clear)
			{
				linha[3*i+1]=linha[3*i+2]=' ';
				continue;
			}
			linha[3*i+1]=tabela[(int)Board.b[l][i].type];
			linha[3*i+2]=tabela2[Board.b[l][i].owner+1];
		}
		linha[3*i]='|';
		linha[3*i+1]=0;
		printf("%s\n",linha);
		linha[0]='|';
		linha[1]='-';
		linha[2]='-';
		if(l==0)
			break;
		for(i=1;i<8;i++)
		{
			linha[3*i]='+';
			linha[3*i+1]=linha[3*i+2]='-';
		}
		linha[3*i]='|';
		linha[3*i+1]=0;
		printf("%s\n",linha);
	}
	linha[0]='*';
	linha[1]=linha[2]='-';
	for(i=1;i<8;i++)
	{
		linha[3*i]='-';
		linha[3*i+1]=linha[3*i+2]='-';
	}
	linha[3*i]='*';
	linha[3*i+1]=0;
	printf("%s\n",linha);
}


#endif


bool ValidateMove(int ol,int oc,int dl,int dc,char turn)
{
	int i,j,di,dj;
	if(Board.b[ol][oc].type==clear || (Board.b[ol][oc].owner)!=turn)
	{
		return false;
	}
	switch(Board.b[ol][oc].type)
	{
	case pawn:
		if(Board.b[dl][dc].type==clear)
		{
			if(dc!=oc)
				return false;
			if((dl-ol)*turn>2 || (dl-ol)*turn<=0)
				return false;
			if(ol!=((7-turn*5)/2) && (dl-ol)*turn==2)
				return false;
			if(ol==(7+turn*5)/2 && (dl-ol)*turn==2 && Board.b[ol+dl/2][oc].type!=clear)
				return false;
		}
		else if((oc-dc)==1 || (oc-dc)==-1)
		{
			if(abs(dl-ol)!=1)
				return false;
		}
		else
			return false;
		break;
	case knight:
		if(abs((ol-dl)*(oc-dc))==2 &&( (Board.b[dl][dc].owner)==-turn || Board.b[dl][dc].type==clear ))
			return true;
		else
			return false;
	case bishop:
		if(abs(oc-dc)!=abs(ol-dl))
			return false;
		if(dc>oc)
			di=1;
		else
			di=-1;
		if(dl>ol)
			dj=1;
		else
			dj=-1;
		for(i=oc+di,j=ol+dj;i!=dc;i+=di, j+=dj)
			if(Board.b[j][i].type)
				return false;
		if(Board.b[dl][dc].owner==turn)
			return false;
		break;
	case king:
		if(abs(oc-dc)==2 && dl==ol)
		{
			if(dc>oc)
				i=1;
			else
				i=-1;
			if(ValidateCastle(i,turn))
				return castle;
			else
				return false;
		}
		if((abs(ol-dl)|abs(oc-dc))!=1)
			return false;
		if(Board.b[dl][dc].owner==turn)
			return false;
		break;
	case hook:
		if(!((oc!=dc) ^ (ol!=dl)))
			return false;
		di=dj=0;
		if(dc>oc)
			di=1;
		else if(dc<oc)
			di=-1;
		if(dl>ol)
			dj=1;
		else if(dl<ol)
			dj=-1;
		for(i=oc+di,j=ol+dj;i!=dc || j!=dl;i+=di, j+=dj)
			if(Board.b[j][i].type)
				return false;
		if(Board.b[dl][dc].owner==turn)
			return false;
		break;
	case queen:
		if(!((oc!=dc) ^ (ol!=dl)) && (abs(oc-dc)!=abs(ol-dl)))
			return false;
		di=dj=0;
		if(dc>oc)
			di=1;
		else if(dc<oc)
			di=-1;
		if(dl>ol)
			dj=1;
		else if(dl<ol)
			dj=-1;
		for(i=oc+di,j=ol+dj;i!=dc || j!=dl;i+=di, j+=dj)
			if(Board.b[j][i].type)
				return false;
		if(Board.b[dl][dc].owner==turn)
			return false;
		break;
	}
	return true;
}

/*bool ValidateMoveEx(char *list,int *n,int ol,int oc,int dl,int dc,char turn)
{
	int i,j,di,dj;
	char r;
	if(Board.b[ol][oc]==0 || (Board.b[ol][oc]>>3)!=turn)
	{
		return false;
	}
	r=1;
	switch(Board.b[ol][oc]&7)
	{
	case pawn:
		if(Board.b[dl][dc]==0)
		{
			if(dc!=oc)
				return false;
			if((dl-ol)*turn>2 || (dl-ol)*turn<=0)
				return false;
			if(ol!=((7-turn*5)/2) && (dl-ol)*turn==2)
				return false;
			if(ol==(7+turn*5)/2 && (dl-ol)*turn==2 && Board.b[(ol+dl/2,oc)]!=0)
				return false;
		}
		else if((oc-dc)==1 || (oc-dc)==-1)
		{
			if(abs(dl-ol)!=1)
				return false;
		}
		else
			return false;
		break;
	case knight:
		if(abs((ol-dl)*(oc-dc))==2 &&( (Board.b[dl][dc].owner)==-turn || Board.b[dl][dc]==0 ))
			return true;
		else
			return false;
	case bishop:
		if(abs(oc-dc)!=abs(ol-dl))
			return false;
		if(dc>oc)
			di=1;
		else
			di=-1;
		if(dl>ol)
			dj=1;
		else
			dj=-1;
		r=1;
		for(i=oc+di,j=ol+dj;i!=dc;i+=di, j+=dj)
		{
			if(Board.b[j][i]!=(queen|(turn<<3)))
				r=2;
			else if(Board.b[j][i])
				return false;
		}
		if(Board.b[dl][dc].owner==turn)
			return false;
		break;
	case king:
		if(abs(oc-dc)==2 && dl==ol)
		{
			if(dc>oc)
				i=1;
			else
				i=-1;
			if(ValidateCastle(i,turn))
				return castle;
			else
				return false;
		}
		if((abs(ol-dl)|abs(oc-dc))!=1)
			return false;
		if(Board.b[dl][dc].owner==turn)
			return false;
		break;
	case hook:
		if(!((oc!=dc) ^ (ol!=dl)))
			return false;
		di=dj=0;
		if(dc>oc)
			di=1;
		else if(dc<oc)
			di=-1;
		if(dl>ol)
			dj=1;
		else if(dl<ol)
			dj=-1;
		for(i=oc+di,j=ol+dj;i!=dc || j!=dl;i+=di, j+=dj)
			if(Board.b[j][i] && Board.b[j][i]!=(queen|(turn<<3)) &&  Board.b[j][i]!=(hook|(turn<<3)) )
				return false;
		if(Board.b[dl][dc].owner==turn)
			return false;
		break;
	case queen:
		if(!((oc!=dc) ^ (ol!=dl)) && (abs(oc-dc)!=abs(ol-dl)))
			return false;
		di=dj=0;
		if(dc>oc)
			di=1;
		else if(dc<oc)
			di=-1;
		if(dl>ol)
			dj=1;
		else if(dl<ol)
			dj=-1;
		for(i=oc+di,j=ol+dj;i!=dc || j!=dl;i+=di, j+=dj)
			if(Board.b[j][i] && ( ( (di==0 || dj ==0) &&  Board.b[j][i]==(hook|(turn<<3)) )  || ( (di!=0 && dj !=0) &&  Board.b[j][i]==(queen|(turn<<3)) ) ) )
				return false;
		if(Board.b[dl][dc].owner==turn)
			return false;
		break;
	}
	return r;
}*/

bool CheckAttack(int l,int c,char turn)
{
	int i,j;
	for(i=0;i<8;i++)
	{
		for(j=0;j<8;j++)
		{
			if(Board.b[i][j].owner==-turn)
				if(ValidateMove(i,j,l,c,-turn))
					return true;
		}
	}
	return false;
}

bool CheckCheck(char turn)
{
	int i,j;
	for(i=0;i<8;i++)
		for(j=0;j<8;j++)
			if(Board.b[i][j].type==king && Board.b[i][j].owner==turn)
				return CheckAttack(i,j,turn);
	return false;
}

bool ValidateCastle(int dir, char turn)
{
	int i,kl,kc;
	kl=(7-turn*7)/2;
	kc=4;
	if(Board.b[kl][kc].type==king && Board.b[kl][kc].owner==turn)
		return false;
	if(CheckAttack(kl,kc,turn))
		return false;
	for(i=kc+dir;i>0 && i<7;i+=dir)
		if(Board.b[kl][i].type || CheckAttack(kl,i,turn))
			return false;
	if(Board.b[kl][i].type==hook && Board.b[kl][i].owner==turn)
		return false;
	return true;
}

int ListMoves(_move *list,char turn)
{
	int n=0;
	int l,c,x,y,i,j;
	_pos r;
	
	for(l=0;l<8;l++)
	{
		for(c=0;c<8;c++)
		{
			if(Board.b[l][c].owner==turn)
			{
				switch(Board.b[l][c].type)
				{
				case pawn:
					y=l+turn;
					x=c;
					if(Board.b[y][x].type==clear)
					{
						put();
						y+=turn;
						if(l==(7-5*turn)/2 && Board.b[y][x].type==clear)
						{
							put();
						}
						y-=turn;
					}
					x++;
					if(x<8 && Board.b[y][x].owner==-turn)
						put();
					x-=2;
					if(x>=0 && Board.b[y][x].owner==-turn)
						put();
					break;
				case knight:
					x=c+2;
					y=l+1;
					if(x<8 && y<8 && Board.b[y][x].owner!=turn)
						put();
					x=c+2;
					y=l-1;
					if(x<8 && y>=0 && Board.b[y][x].owner!=turn)
						put();
					x=c+1;
					y=l+2;
					if(x<8 && y<8 && Board.b[y][x].owner!=turn)
						put();
					x=c+1;
					y=l-2;
					if(x<8 && y>=0 && Board.b[y][x].owner!=turn)
						put();
					x=c-1;
					y=l+2;
					if(x>=0 && y<8 && Board.b[y][x].owner!=turn)
						put();
					x=c-1;
					y=l-2;
					if(x>=0 && y>=0 && Board.b[y][x].owner!=turn)
						put();
					x=c-2;
					y=l+1;
					if(x>=0 && y<8 && Board.b[y][x].owner!=turn)
						put();
					x=c-2;
					y=l-1;
					if(x>=0 && y<8 && Board.b[y][x].owner!=turn)
						put();
					break;
				case hook:
				case queen:
					x=c;y=l;
					for(x++;Board.b[y][x].type==clear && x<8;x++)
						put();
					if(x<8 && Board.b[y][x].owner==-turn)
						put();
					x=c;y=l;
					for(x--;Board.b[y][x].type==clear && x>=0;x--)
						put();
					if(x>=0 && Board.b[y][x].owner==-turn)
						put();
					x=c;y=l;
					for(y++;Board.b[y][x].type==clear && y<8;y++)
						put();
					if(y<8 && Board.b[y][x].owner==-turn)
						put();
					x=c;y=l;
					for(y--;Board.b[y][x].type==clear && y>=0;y--)
						put();
					if(y>=0 && Board.b[y][x].owner==-turn)
						put();
					if((Board.b[l][c].type)==hook)
						break;
				case bishop:
					x=c;y=l;
					for(x++,y++;x<8 && y<8 && Board.b[y][x].type==clear;x++,y++)
						put();
					if(x<8 && y<8 && Board.b[y][x].owner==-turn)
						put();
					x=c;y=l;
					for(x++,y--;x<8 && y>=0 && Board.b[y][x].type==clear;x++,y--)
						put();
					if(x<8 && y>=0 && Board.b[y][x].owner==-turn)
						put();
					x=c;y=l;
					for(x--,y--;x>=0 && y>=0 && Board.b[y][x].type==0;x--,y--)
						put();
					if(x>=0 && y>=0 && Board.b[y][x].owner==-turn)
						put();
					x=c;y=l;
					for(x--,y++;x>=0 && y<8 && Board.b[y][x].type==0;x--,y++)
						put();
					if(x<=0 && y>=0 && Board.b[y][x].owner==-turn)
						put();
					break;
				case king:
					x=c+1;y=l+1;
					if(x<8 && y<8 && Board.b[y][x].owner!=turn)
						put();
					x=c;y=l+1;
					if(y<8 && Board.b[y][x].owner!=turn)
						put();
					x=c-1;y=l+1;
					if(x>=0 && y<8 && Board.b[y][x].owner!=turn)
						put();
					x=c-1;y=l;
					if(x>=0 && Board.b[y][x].owner!=turn)
						put();
					x=c-1;y=l-1;
					if(x>=0 && y>=0 && Board.b[y][x].owner!=turn)
						put();
					x=c;y=l-1;
					if(y>=0 && Board.b[y][x].owner!=turn)
						put();
					x=c+1;y=l-1;
					if(x<8 && y>=0 && Board.b[y][x].owner!=turn)
						put();
					x=c+1;y=l;
					if(c<8 && Board.b[y][x].owner!=turn && Board.b[y][x].owner!=turn)
						put();
					if(ValidateCastle(1,turn))
					{
						y=l;
						x=c+2;
						put();
					}
					if(ValidateCastle(-1,turn))
					{
						y=l;
						x=c-2;
						put();
					}
					break;
				}
			}
		}
	}
	for(i=0,j=0;i<n;i++)
	{
		r=Board.b[(int)list[i].dl][list[i].dc];
		Board.b[(int)list[i].dl][list[i].dc]= Board.b[(int)list[i].ol][list[i].oc];
		Board.b[(int)list[i].ol][list[i].oc].type=clear;
		if(!CheckCheck(turn))
		{
			list[j]=list[i];
			j++;
		}
		Board.b[(int)list[i].ol][list[i].oc]= Board.b[(int)list[i].dl][list[i].dc];
		Board.b[(int)list[i].dl][list[i].dc]=r;
	}
	n=j;
	return n;
}

int Compute(char turn,_move *best,int depth)
{
	int n,i,t,mj;
	_pos c;
	_move list[150];
	_move d;
	n=ListMoves(list,turn);
	if(n==0)
	{
		if(CheckCheck(turn))
		{
			return -1000000*turn;
		}
		else
		{
			return 0;
		}
	}
	if(depth==0)
		return EvaluatePos();
	mj=-1000000;
	for(i=0;i<n;i++)
	{
		c=Board.b[list[i].dl][list[i].dc];
		Board.b[list[i].dl][list[i].dc]=Board.b[list[i].ol][list[i].oc];
		Board.b[list[i].ol][list[i].oc].type=clear;
		if(Board.b[list[i].ol][list[i].dl].type==king && Board.b[list[i].ol][list[i].dl].owner==turn && abs(list[i].oc-list[i].dc)==2)
		{
			if(list[i].oc>list[i].dc)
			{
				Board.b[list[i].dl][0]=Board.b[list[i].ol][3];
				Board.b[list[i].ol][3].type=clear;
			}
			else
			{
				Board.b[list[i].dl][7]=Board.b[list[i].ol][5];
				Board.b[list[i].ol][5].type=clear;
			}
			t=Compute(-turn,&d,depth-1);
			if(t*turn >mj)
			{
				mj=t*turn;
				*best=list[i];
			}
			if(list[i].oc>list[i].dc)
			{
				Board.b[list[i].ol][3]=Board.b[list[i].dl][0];
				Board.b[list[i].dl][0].type=clear;
			}
			else
			{
				Board.b[list[i].ol][5]=Board.b[list[i].dl][7];
				Board.b[list[i].dl][7].type=clear;
			}
		}
		else
		{
			t=Compute(-turn,&d,depth-1);
			if(t*turn >mj)
			{
				mj=t*turn;
				*best=list[i];
			}
		}
		Board.b[list[i].ol][list[i].oc]=Board.b[list[i].dl][list[i].dc];
		Board.b[list[i].dl][list[i].dc]=c;
	}
	return mj*turn;
}

int EvaluatePos()
{
	int x,y,v;
	for(x=0,v=0;x<8;x++)
	{
		for(y=0;y<8;y++)
		{
			v+=valtable[(int)Board.b[x][y].type]*(Board.b[x][y].owner);
		}
	}
	return v;
}
