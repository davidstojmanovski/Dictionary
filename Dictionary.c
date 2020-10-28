#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#define MAX_WORD_LEN 64
#define LETTERS 26









typedef struct trie_node
{
	char c;
	int term;
	int subwords;
	struct trie_node *parent;
	struct trie_node *children[LETTERS];
	pthread_mutex_t lock;
} trie_node;


struct trie_node* head;



typedef struct scanned_file
{
	char file_name[256];
	time_t mod_time;
	char dir_name[256];
} scanned_file;




typedef struct pomocna{
	char pok[256];
}pomocna;



struct scanned_file nizFajlova[300];
int brojacFileova=0;



void ubaciRec(char *rec){
	char pomagac[64];
	int cnt=0;
	char ch=*rec;


	while(ch!='\0'){

		pomagac[cnt]=ch;
		cnt++;

		rec++;
		ch=*rec;
	}

	cnt--;

	trie_node *cvor;
	cvor=head;
	int zakljucan=0;

	if(head->children[(int)pomagac[0]-97]!=NULL){
		cvor=head->children[(int)pomagac[0]-97];
		pthread_mutex_lock(&cvor->lock);
		cvor=head;
		zakljucan=1;

	}
	for(int i=0; i<=cnt; i++){

		if(cvor->children[(int)pomagac[i]-97]==NULL){

			trie_node *novi=(trie_node*)malloc(sizeof(trie_node));
			for(int j=0; j<26; j++){
				novi->children[j]=NULL;
			}
			novi->subwords=0;
			pthread_mutex_init(&novi->lock,NULL);
			cvor->children[(int)pomagac[i]-97]=novi;
			novi->parent=cvor;
			novi->c=pomagac[i];

			if(i==cnt){

			 novi->term=1;
			while(cvor!=NULL){
				cvor=cvor->parent;
				if(cvor!=NULL){
				cvor->subwords=cvor->subwords+1;
					}
			}

			}
			else novi->term=0;
			cvor=novi;
		}
		if(!(cvor->children[(int)pomagac[i]-97]==NULL)){
			cvor=cvor->children[(int)pomagac[i]-97];
		}
	}
	if(zakljucan==1){
		cvor=head->children[(int)pomagac[0]-97];
		pthread_mutex_unlock(&cvor->lock);

	}


}


void citanjeFajla(char *filePath){
	FILE *fp;
	int ch;

	fp=fopen(filePath,"r");
	int novaRec=1;
	char rec[64];
	int brojac=0;
	if(fp){
		while((ch=getc(fp))!= EOF){

			if(ch==' '|| ch=='\t' || ch=='\n'){

				rec[brojac]='\0';

				int dalJeValidno=1;
				for(int i=0; i<brojac; i++){
					if((int)rec[i]<97 || (int)rec[i]>122) dalJeValidno=0;
				}

				if(dalJeValidno)ubaciRec(rec);

				rec[0]=0;
				brojac=0;
			}
			else{
				rec[brojac]=ch;
				brojac++;
			}



		}
		fclose(fp);
	}



}

typedef struct search_result
{
int result_count;
char **words;
} search_result;




void resenje(char *pre, trie_node* root, char str[], int lvl){

	if(root->term==1){
		str[lvl]='\0';
		printf("%s",pre );
		printf("%s\n",str);
	}
	int i;
	for(i=0; i<26; i++){
		if(root->children[i]){
			str[lvl]=i+'a';
			resenje(pre,root->children[i],str,lvl+1);
		}
	}

}


/*search_result **/ void trie_get_words(char *prefiks){

	int k=strlen(prefiks);



	trie_node *prob;
	trie_node *nob;
	prob=head;
	nob=head;
	int zakljucan=0;
	char ch=*prefiks;
	char *pre=NULL;
	pre=prefiks;


	if(nob->children[((int)(*prefiks))-97]!=NULL){
		nob=head->children[((int)(*prefiks))-97];
		zakljucan=1;


		pthread_mutex_lock(&nob->lock);
	}
	int radi=1;
	for(int i=0; i<k; i++){
		if(prob->children[((int)(*prefiks))-97]==NULL){
			printf("nema reci sa ovim prefiksom\n");
			pthread_mutex_unlock(&nob->lock);
			radi=0;
			break;
		}
		prob=prob->children[((int)(*prefiks))-97];

		prefiks++;

	}


	if(radi){





		printf("%d\n",prob->subwords);

		char stringic[30];
		int level=0;
		resenje(pre,prob,stringic,level);
		if(zakljucan){


		pthread_mutex_unlock(&nob->lock);
	}


	}


}





void* skener(void *ptr)
{



	struct pomocna *poruka=(struct pomocna *)ptr;
    char *message;
    message=(char*)poruka->pok;

    DIR *dir;

    struct dirent *sd;
    while(1){
	    dir=opendir((char*)poruka->pok);

	    if(dir==NULL){
	    	printf("Ne moze da se otvori dati direktorijum\n");
	    	exit(1);
	    }

	    while( (sd=readdir(dir)) !=NULL ){
			int postoji=0;
			int i;
			int izmenjen=0;
			for(i=0; i<brojacFileova; i++){
					if(strcmp(nizFajlova[i].file_name,sd->d_name)==0) {


						postoji=1;


				char kutanja[30];


				strcpy(kutanja,message);
				strcat(kutanja,"/");
				strcat(kutanja,nizFajlova[i].file_name);

				struct stat path_star;
				stat(kutanja,&path_star);

				if(nizFajlova[i].mod_time<path_star.st_mtime){
					printf("izmenili smo %s\n" ,nizFajlova[i].file_name);

					nizFajlova[i].mod_time=path_star.st_mtime;
					//corupted file
					citanjeFajla(kutanja);


					}

					}


				}




			if(postoji==0 ){
				if(postoji==0 ){
				struct scanned_file newStruct;
				strcpy(newStruct.file_name,sd->d_name);

				strcpy(newStruct.dir_name,(char*)poruka->pok);




				char* putanja=newStruct.dir_name;

				strcat(putanja,"/");
				strcat(putanja,newStruct.file_name);


				struct stat path_stat;
				stat(putanja,&path_stat);



				if(!(S_ISDIR(path_stat.st_mode))){

				citanjeFajla(putanja);
				nizFajlova[brojacFileova]=newStruct;
					newStruct.mod_time=path_stat.st_mtime;
				brojacFileova++;
				printf(" ubacili smo %s\n", nizFajlova[brojacFileova-1].file_name );
				printf("%d\n",head->subwords );
				}


				}



			}
	    }


	    closedir(dir);
	    usleep(60000);
   }
}



int main(int argc, char *argv[])
{
	head =(trie_node*)malloc(sizeof(trie_node));
	for(int k=0; k<26; k++){
		head->children[k]=NULL;
	}
	head->term=1;
	head->c='-';
	head->subwords=0;
	head->parent=NULL;





	pthread_t threads[50];
	int brojacTredova=1;
	struct pomocna niz[20];
	int brojac=0;





	while(1){
		char input[100]={0};
		char add[]="_add_ ";
		char end[]="_stop_";

		scanf("%s",input);




		if((strncmp(input,add,5))==0){
			int i=5;
			int j=2;

			char target[15]={0};
			target[0]='.';
			target[1]='/';
			while(input[i]!='\0'){
				target[j]=input[i];
				j++;
				i++;
			}


			strcpy(niz[brojac].pok,target);

			pthread_create(&threads[brojacTredova],NULL,skener, niz+brojac);
			brojacTredova=brojacTredova+1;
			brojac++;

			}





		else if((strncmp(input,end,6))==0){
			break;
		}



			else{
			int k=strlen(input);

			trie_get_words(input);
		}

	}


}
