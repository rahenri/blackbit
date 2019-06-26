#include "main.h"

_board Board;
_move list[150];
_peca peca[32];
int npeca;
int turn;
int eval;
int main(void)
{
	char mov[128];
	int oc,ol,dc,dl,r,i,j,wp;
	char p[3];
	FILE *f, *saida = stdout;
	_move best[128];
	_pos b;
	while (1) {
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
				else if(strcmp(mov,"recover")==0)
				{
					f=fopen("back.txt","r");
					fscanf(f,"%d",&turn);
					for(i=0;i<8;i++)
					{
						for(j=0;j<8;j++)
						{
							fscanf(f,"%d",&Board.b[i][j].id);
							fscanf(f,"%d",&Board.b[i][j].owner);
							fscanf(f,"%d",&Board.b[i][j].type);
						}
					}
					fscanf(f,"%d",&npeca);
					for(i=0;i<32;i++)
					{
						fscanf(f,"%d %d %d %d",&peca[i].c,&peca[i].l,&peca[i].owner,&peca[i].type);
					}
					fclose(f);
					PrintBoard();
					continue;

				}
				else if(strcmp("comp",mov)==0)
				{
					p[turn+1]='C';
					continue;
				}
				oc=mov[0]-'a';
				ol=mov[1]-'1';
				dc=mov[2]-'a';
				dl=mov[3]-'1';
				if(vl(ol) || vl(oc) || vl(dc) || vl(dl) || !(r=ValidateMove(ol,oc,dl,dc,turn))) {
					fprintf(saida, "movimento inválido");
					continue;
				}
			}
			else
			{
				fprintf(saida,"Pensando...\n");
				r=ComputerPlay(turn,best,4);
				fprintf(saida,"%+d\n",r);
				ol=best[0].ol;
				oc=best[0].oc;
				dl=best[0].dl;
				dc=best[0].dc;
				fprintf(saida,"%c%c%c%c\n",best[0].oc+'a',best[0].ol+'0',best[0].dc+'a',best[0].dl+'0');
			}
			move(ol,oc,dl,dc);
			if(Board.b[dl][dc].type==king && abs(oc-dc)==2)
				r=2;
			else
				r=0;
			if(r==castle)
			{
				if(dc>oc)
				{
					movex(dl,7,dl,5);
				}
				else
				{
					movex(dl,0,dl,3);
				}
			}
			if(CheckCheck(turn))
			{
				volta(ol,oc,dl,dc);
				fprintf(saida,"movimento inválido");
			}
			turn=-turn;
			if(CheckCheck(turn))
			{
				if(ListMoves(list,turn)==0)
				{
					fprintf(saida,"Mate!\n");
					PrintBoard();
					break;
				}
				else
				{
					fprintf(saida,"Cheque!\n");
				}
			}
			PrintBoard();

			//Back up
			f=fopen("back.txt","w");
			fprintf(f,"%d ",(int)turn);
			for(i=0;i<8;i++)
			{
				for(j=0;j<8;j++)
				{
					fprintf(f,"%d ",Board.b[i][j].id);
					fprintf(f,"%d ",Board.b[i][j].owner);
					fprintf(f,"%d ",Board.b[i][j].type);
				}
			}
			fprintf(f,"%d ",npeca);
			for(i=0;i<32;i++)
			{
				fprintf(f,"%d %d %d %d ",peca[i].c,peca[i].l,peca[i].owner,peca[i].type);
			}
			fprintf(f,"\n");
			fclose(f);
			continue;
		}
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
	int i,l,c;
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
		Board.b[6][i].owner=black;
	}
	npeca=0;
	for(l=0;l<8;l++)
	{
		for(c=0;c<8;c++)
		{
			if(Board.b[l][c].type)
			{
				peca[npeca].l=l;
				peca[npeca].c=c;
				peca[npeca].owner=Board.b[l][c].owner;
				peca[npeca].type=Board.b[l][c].type;
				Board.b[l][c].id=npeca;
				npeca++;
			}
		}
	}
}

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
		if(Board.b[dl][dc].owner==turn && Board.b[dl][dc].type!=clear)
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
		if(Board.b[dl][dc].owner==turn && Board.b[dl][dc].type!=clear)
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
		if(Board.b[dl][dc].type!=clear && Board.b[dl][dc].owner==turn)
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
		if(Board.b[dl][dc].owner==turn && Board.b[dl][dc].type!=clear)
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
	int i;
	for(i=0;i<npeca;i++)
	{
		if(peca[i].owner==-turn)
			if(ValidateMove(peca[i].l,peca[i].c,l,c,-turn))
				return true;
	}
	return false;
}

bool CheckCheck(char turn)
{
	int i;
	for(i=0;i<npeca;i++)
		if(peca[i].type==king && peca[i].owner==turn)
				return CheckAttack(peca[i].l,peca[i].c,turn);
	return false;
}

bool ValidateCastle(int dir, char turn)
{
	int i,kl,kc;
	kl=(7-turn*7)/2;
	kc=4;
	if(Board.b[kl][kc].type!=king || Board.b[kl][kc].owner!=turn)
		return false;
	if(Board.b[kl][(7+7*dir)/2].type!=hook || Board.b[kl][(7+7*dir)/2].owner!=turn)
		return false;
	for(i=kc+dir;i>0 && i<7;i+=dir)
		if(Board.b[kl][i].type)
			return false;
	if(CheckAttack(kl,kc,turn))
		return false;
	for(i=kc+dir;i>0 && i<7;i+=dir)
		if(CheckAttack(kl,i,turn))
			return false;
	return true;
}

int ListMoves(_move *list,char turn)
{
	int n=0;
	int l,c,x,y,i,j,k,wp;
	_pos b;
	
	for(k=0;k<npeca;k++)
	{
		if(peca[k].owner==turn)
		{
			l=peca[k].l;
			c=peca[k].c;
			switch(peca[k].type)
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
				if(x<8 && Board.b[y][x].type!=0 && Board.b[y][x].owner==-turn)
					put();
				x-=2;
				if(x>=0 && Board.b[y][x].type!=0 && Board.b[y][x].owner==-turn)
					put();
				break;
			case knight:
				x=c+2;
				y=l+1;
				if(x<8 && y<8 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
					put();
				x=c+2;
				y=l-1;
				if(x<8 && y>=0 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
					put();
				x=c+1;
				y=l+2;
				if(x<8 && y<8 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
					put();
				x=c+1;
				y=l-2;
				if(x<8 && y>=0 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
					put();
				x=c-1;
				y=l+2;
				if(x>=0 && y<8 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
					put();
				x=c-1;
				y=l-2;
				if(x>=0 && y>=0 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
					put();
				x=c-2;
				y=l+1;
				if(x>=0 && y<8 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
					put();
				x=c-2;
				y=l-1;
				if(x>=0 && y>=8 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
					put();
				break;
			case hook:
			case queen:
				x=c;y=l;
				for(x++;x<8 && Board.b[y][x].type==clear;x++)
					put();
				if(x<8 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
					put();
				x=c;y=l;
				for(x--;x>=0 && Board.b[y][x].type==clear;x--)
					put();
				if(x>=0 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
					put();
				x=c;y=l;
				for(y++;y<8 && Board.b[y][x].type==clear;y++)
					put();
				if(y<8 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
					put();
				x=c;y=l;
				for(y--;y>=0 && Board.b[y][x].type==clear;y--)
					put();
				if(y>=0 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
					put();
				if((Board.b[l][c].type)==hook)
					break;
			case bishop:
				x=c;y=l;
				for(x++,y++;x<8 && y<8 && Board.b[y][x].type==clear;x++,y++)
					put();
				if(x<8 && y<8 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
					put();
				x=c;y=l;
				for(x++,y--;x<8 && y>=0 && Board.b[y][x].type==clear;x++,y--)
					put();
				if(x<8 && y>=0 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
					put();
				x=c;y=l;
				for(x--,y--;x>=0 && y>=0 && Board.b[y][x].type==clear;x--,y--)
					put();
				if(x>=0 && y>=0 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
					put();
				x=c;y=l;
				for(x--,y++;x>=0 && y<8 && Board.b[y][x].type==clear;x--,y++)
					put();
				if(x>=0 && y<8 && Board.b[y][x].type && Board.b[y][x].owner==-turn)
					put();
				break;
			case king:
				x=c+1;y=l+1;
				if(x<8 && y<8 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
					put();
				x=c;y=l+1;
				if(y<8 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
					put();
				x=c-1;y=l+1;
				if(x>=0 && y<8 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
					put();
				x=c-1;y=l;
				if(x>=0 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
					put();
				x=c-1;y=l-1;
				if(x>=0 && y>=0 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
					put();
				x=c;y=l-1;
				if(y>=0 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
					put();
				x=c+1;y=l-1;
				if(x<8 && y>=0 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
					put();
				x=c+1;y=l;
				if(c<8 && (Board.b[y][x].type==clear || Board.b[y][x].owner!=turn))
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
	for(i=0,j=0;i<n;i++)
	{
		move(list[i].ol,list[i].oc,list[i].dl,list[i].dc);
		if(!CheckCheck(turn))
		{
			list[j]=list[i];
			j++;
		}
		volta(list[i].ol,list[i].oc,list[i].dl,list[i].dc);
	}
	n=j;
	return n;
}

int ComputerPlay(char turn,_move *best,int depth)
{
	eval=EvaluatePos();
	return Compute(turn,best,depth);
}

int Compute(char turn,_move *best,int depth)
{
	int n,i,t,mj,j,wp;
	_pos b;
	_move list[150];
	_move d[128];
	n=ListMoves(list,turn);
	if(n==0)
	{
		if(CheckCheck(turn))
		{
			return -100000000*turn;
		}
		else
		{
			return 0;
		}
	}
	mj=-1000000000;
	for(i=0;i<n;i++)
	{
		move(list[i].ol,list[i].oc,list[i].dl,list[i].dc);
		eval-=valtable[b.type]*b.owner;
		CheckBoard();
		if(Board.b[list[i].dl][list[i].dc].type==king && Board.b[list[i].dl][list[i].dc].owner==turn && abs(list[i].oc-list[i].dc)==2)
		{
			if(list[i].oc>list[i].dc)
			{
				movex(list[i].ol,0,list[i].ol,3);
			}
			else
			{
				movex(list[i].ol,7,list[i].ol,5);
			}
			if(depth==1)
			{
				t=eval;
			}
			else
			{
				t=Compute(-turn,d,depth-1);
			}
			if(t*turn >mj)
			{
				mj=t*turn;
				*best=list[i];
				//memcpy(best+1,d,sizeof(_move)*3);
			}
			if(list[i].oc>list[i].dc)
			{
				movex(list[i].ol,3,list[i].ol,0);
			}
			else
			{
				movex(list[i].ol,5,list[i].ol,7);
			}
		}
		else
		{
			if(depth==1)
			{
				t=eval;
			}
			else
			{
				t=Compute(-turn,d,depth-1);
			}
			if(t*turn >mj)
			{
				mj=t*turn;
				*best=list[i];
				//memcpy(best+1,d,sizeof(_move)*3);				
			}
		}
		volta(list[i].ol,list[i].oc,list[i].dl,list[i].dc);
		eval+=valtable[b.type]*b.owner;
		CheckBoard();
/*		if(depth==4)
		{
			printf("%c%c%c%c ",list[i].oc+'a',list[i].ol+'1',list[i].dc+'a',list[i].dl+'1');
			for(j=0;j<3;j++)
			{
				printf("%c%c%c%c ",d[j].oc+'a',d[j].ol+'1',d[j].dc+'a',d[j].dl+'1');
			}
			printf("%+d\n",t);
		}*/
	}
	return mj*turn;
}

int EvaluatePos()
{
	int i,v;
	for(i=0,v=0;i<npeca;i++)
	{
		v+=valtable[(int)peca[i].type]*(peca[i].owner);
	}
	return v;
}

void CheckBoard()
{
	int i,j,n;
	for(i=0,n=0;i<8;i++)
	{
		for(j=0;j<8;j++)
		{
			if(Board.b[i][j].type)
				n++;
		}
	}
	if(n!=npeca)
	{
		printf("Contagem não bate\n");
#ifdef O3
		quit(0);
#else
		return;
#endif
	}
	for(i=0;i<npeca;i++)
	{
		if(Board.b[peca[i].l][peca[i].c].type!=peca[i].type || Board.b[peca[i].l][peca[i].c].owner!=peca[i].owner || Board.b[peca[i].l][peca[i].c].id!=i)
		{
			printf("(%d,%d) -> peca(type=%d,owner=%d,id=%d) Board(type=%d,owner=%d,id=%d)\n",peca[i].l,peca[i].c,peca[i].type,peca[i].owner,i,Board.b[peca[i].l][peca[i].c].type,Board.b[peca[i].l][peca[i].c].owner,Board.b[peca[i].l][peca[i].c].id);
#ifdef O3
			quit(0);
#else
			return;
#endif
		}
	}
}
