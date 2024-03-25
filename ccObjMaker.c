#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* build =   gcc -o fwObjmaker fwObjmaker.c*/
/*  test  */
struct dbItem
{
	char fieldName[124];
	char type[124];
	char extra[124];
};

void replaceSubstr(char *line, const char *search, const char *replace)
{
     char *sp;

     if ((sp = strstr(line, search)) == NULL) {
         return;
     }
     int search_len = strlen(search);
     int replace_len = strlen(replace);
     int tail_len = strlen(sp+search_len);

     memmove(sp+replace_len,sp+search_len,tail_len+1);
     memcpy(sp, replace, replace_len);
}

void addSpacesToCapitals(char *line){
    char sp[200] = {0};
    int i,x;
    i = 0;
    x = 0;
    int lineLen = strlen(line);
    for(i=0;i<lineLen;i++){
        if(line[i]>='A' && line[i] <= 'Z'){
            // insert space before.
            if(x>0){
                sp[x] = ' ';
                x = x + 1;
            }
            sp[x] = line[i];
        }else{
            sp[x] = line[i];
        }
        x = x + 1;
    }
    sp[x] = '\0';
    
    memmove(line, sp,strlen(sp)+1);
    memcpy(line, sp, strlen(sp)+1);
}

void getDescriptionField(char tblname[],char * ret){
        
    char tstring[128];
    char res[128];
    int cresult;

    strcpy(res,"Description");

    strcpy(tstring,"ClientGroup");
    cresult = strcmp(tblname, tstring);
    if(cresult==0){
        strcpy(res,"GroupDescription");
    }

    strcpy(tstring,"Client");
    cresult = strcmp(tblname, tstring);
    if(cresult==0){
        strcpy(res,"clientName");
    }

    strcpy(tstring,"User");
    cresult = strcmp(tblname, tstring);
    if(cresult==0){
        strcpy(res,"firstname");
    }        
    strcpy(tstring,"Project");
    cresult = strcmp(tblname, tstring);
    if(cresult==0){
        strcpy(res,"title");
    }        
    strcpy(tstring,"UserForm");
    cresult = strcmp(tblname, tstring);
    if(cresult==0){
        strcpy(res,"FormName");
    }        

    memmove(ret, res,strlen(res));
    memcpy(ret, res, strlen(res));
}


int main(int argc, char *argv[]){
	if(argc < 2){
		printf("too few arguments\nUsage: %s objectName csvfileforFields\n",argv[0]);
		printf("csv file format is: \n a) no headers\n b) fieldname; type and size definition; anything extra \n");
		printf("EG: carType; INT; NULL DEFAULT 0;1\n");
		printf("note the primary key/autoincrement will be added automatically so do not include\n");
		printf("the very last column is used to define if something is a text field or lookup.  0=text field, 1=select, 2=yes no\n");
        printf("if using python add the word python as the last argument\n");
	}
    /*int runpython = 0;
	int runsqlite = 0;
    if(argc >3){
        if(strcmp(argv[3],"python")==0){
            runpython = 1;
        }else if(strcmp(argv[3],"sqlite")==0){
			runsqlite = 1;
		}
    }*/

	char fields[128][128];
	char types[128][128];
	char extra[128][128];
	char feditor[128][128]; /// 0 is text box, 1 is select, 2 is checkbox
	char lookuptable[128][128]; /// 0 is text box, 1 is select, 2 is checkbox

	FILE *file = fopen(argv[2],"r");
	char *line = NULL;
	size_t len = 0;
	size_t read;

    /// these variables are used for doing the split on line
	int charcount = 0;
	int charcountforCol = 0;
	char ch;
	int	curCol = 0;
	int totRows = 0;
    //	maxlencol = 128;
    int compresult = 0;
    int tp = 115;
    char teststring[128];
	char comma = ' ';

    int posss = 0;
	if(file==NULL){
		exit(EXIT_FAILURE);
	}	
	while ((read = getline(&line, &len, file)) != -1){
		curCol = 0;
		charcountforCol = 0;
		charcount = 0;

    //		printf("got line of length: %zu :\n",read);
    //		printf("CREATE TABLE IF NOT EXISTS tbl%s(\n",argv[1]);
    //		printf("	%sID	INT NOT NULL AUTO_INCREMENT,\n", argv[1]);
		ch = line[charcount];
		while(ch !='\n' && charcount != len){
			if(totRows>128){
				printf("too many columns 128 maximum");
				exit(EXIT_FAILURE);
			}
    //			printf("%i",charcount);
			if(ch != ';'){
				if(curCol==0){
					fields[totRows][charcountforCol]=ch;
                    feditor[totRows][0] = 0;
				}else if(curCol==1){
					types[totRows][charcountforCol]=ch;
				}else if(curCol==2){
					extra[totRows][charcountforCol]=ch;
				}else if(curCol==3){
					feditor[totRows][charcountforCol]=ch;
				}else if(curCol==4){
					lookuptable[totRows][charcountforCol]=ch;
				}
				charcountforCol++;
				
			}else{
				if(curCol==0){
					printf("%s\n",fields[totRows]);
				}
				curCol++;
				charcountforCol = 0;
			}
			charcount++;
			ch=line[charcount];
		}
		
		totRows++;
	}

	fclose(file);
	if(line)
		free(line);

    char ObjectName[128];
    strcpy(ObjectName,argv[1]);
    //int olen = strlen(ObjectName);
    //ObjectName[olen] = '\0';
    replaceSubstr(ObjectName, "Custom_", "");
    printf("%s\n",ObjectName); 
    addSpacesToCapitals(ObjectName);
    printf("%s\n",ObjectName); 

    printf("\n");
    //// step 
    //// CREATE mysql file

    char filename[128];
    snprintf(filename, sizeof filename, "%s.mysql",argv[1]);


    FILE *fsql;
    fsql = fopen(filename,"w");

    int count = 0;
    int hasBIN = 0;

    /// CREATE TABLE first
    if(fsql!=NULL){
        fprintf(fsql,"/**************************************** \n");
        fprintf(fsql,"      %s \n\n",argv[1]);
        fprintf(fsql,"****************************************/ \n");
        fprintf(fsql,"\nDELIMITER ; \n\n");
        fprintf(fsql,"CREATE TABLE IF NOT EXISTS tbl%s(\n",argv[1]);
        fprintf(fsql,"\t%sID\t\tINT NOT NULL AUTO_INCREMENT,\n",argv[1]);
        for(count==0;count<totRows;count++){
            fprintf(fsql,"\t%s\t\t%s %s,\n",fields[count],types[count],extra[count]);
            
            strcpy(teststring,"VARBINARY");
            compresult = strncmp(types[count], teststring,9);
            if(compresult==0){
                hasBIN=1;
            }
        }
        fprintf(fsql,"\tRecordDeleted\t\tINT DEFAULT 0,\n");
        fprintf(fsql,"\tRecordLockByUserID\tINT DEFAULT 0,\n");
        fprintf(fsql,"\tRecordLockTime\t\tDATETIME NULL,\n");
        fprintf(fsql,"\tdateCreated\t\tDATETIME DEFAULT CURRENT_TIMESTAMP,\n");
        fprintf(fsql,"\tdateModified\t\tDATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\n");
        fprintf(fsql,"\tModifiedByUserID\tINT DEFAULT 0,\n");
        if(hasBIN==1){
            fprintf(fsql,"\tEIV\tVARBINARY(255),\n");
            fprintf(fsql,"\tES\tVARBINARY(255),\n");
            fprintf(fsql,"\tEVersion\tINT,\n");
        }
        fprintf(fsql,"\tPRIMARY KEY(%sID)\n",argv[1]);
        fprintf(fsql,");\n");
        

    /// now we create the procs
    //  sp_update proc
        count=0;
        fprintf(fsql,"\nDELIMITER // \n\n");
        fprintf(fsql,"DROP PROCEDURE IF EXISTS sp_update%s;\n\n", argv[1]);
        fprintf(fsql,"CREATE PROCEDURE sp_update%s(\n",argv[1]);
        fprintf(fsql,"\tv%sID\t\tINT,\n",argv[1]);
        for(count==0;count<totRows;count++){
            if(count==totRows-1){
                //fprintf(fsql,"	v%s	%s\n",fields[count],types[count]);
                // have added currentuser at the end so no need to look for comma
                fprintf(fsql,"\tv%s\t%s,\n",fields[count],types[count]);
            }else{
                fprintf(fsql,"\tv%s\t%s,\n",fields[count],types[count]);
            }
        }
        fprintf(fsql,"\tvCurrentUserID\tINT\n");
        if(hasBIN==1){
            fprintf(fsql,"\t,vKey\tVARCHAR(1000)\n");
        }
        count = 0;
        fprintf(fsql,")\n");
        fprintf(fsql,"BEGIN\n");
        fprintf(fsql,"\tDECLARE v_count INT;\n\n");
        fprintf(fsql,"\tSET v_count=(SELECT count(%sID) FROM tbl%s where %sID=v%sID);\n",argv[1],argv[1],argv[1],argv[1]);
        fprintf(fsql,"\tIF(v_count > 0) THEN\n");
        fprintf(fsql,"\t\tUPDATE tbl%s SET\n",argv[1]);
        compresult = 0;
        tp=0;
        for(count==0;count<totRows;count++){

            strcpy(teststring,"VARBINARY");
            compresult = strncmp(types[count], teststring,9);
            tp=2;
            if(compresult==0){
                tp=6;
            }
            if(count==totRows-1){
                //fprintf(fsql,"		%s=v%s\n",fields[count],fields[count]);	
                if(tp==6){
                    fprintf(fsql,"\t\t%s=setEDataHKDF(v%s,vKey,EIV,ES),\n",fields[count],fields[count]);	
                }else{
                    fprintf(fsql,"\t\t%s=v%s,\n",fields[count],fields[count]);	
                }
            }else{
                if(tp==6){
                    fprintf(fsql,"\t\t%s=setEDataHKDF(v%s,vKey,EIV,ES),\n",fields[count],fields[count]);
                }else{
                    fprintf(fsql,"\t\t%s=v%s,\n",fields[count],fields[count]);
                }    
            }
        }
        fprintf(fsql,"\t\tModifiedByUserID=vCurrentUserID\n");	
        fprintf(fsql,"\t\tWHERE %sID = v%sID;\n",argv[1],argv[1]);
        fprintf(fsql,"\t\tselect v%sID as %sID;\n",argv[1],argv[1]);

        count=0;	
        fprintf(fsql,"\tELSE\n\n");
        if(hasBIN==1){
            fprintf(fsql,"\t\tSET @iv=RANDOM_BYTES(16);\n\n");
            fprintf(fsql,"\t\tSET @salt=RANDOM_BYTES(16);\n\n");
        }
        fprintf(fsql,"\t\tINSERT INTO tbl%s(\n",argv[1]);
        for(count==0;count<totRows;count++){
            strcpy(teststring,"VARBINARY");
            compresult = strncmp(types[count], teststring,9);
            tp=2;
            if(compresult==0){
                tp=6;
            }
            if(count==totRows-1){
                //fprintf(fsql,"		%s\n",fields[count]);	
                fprintf(fsql,"\t\t\t%s,\n",fields[count]);	
            }else{
                fprintf(fsql,"\t\t\t%s,\n",fields[count]);	
            }
        }
        fprintf(fsql,"\t\t\tModifiedByUserID\n");	
        if(hasBIN==1){
            fprintf(fsql,"\t\t,EIV,\n\n");
            fprintf(fsql,"\t\tES,\n\n");
            fprintf(fsql,"\t\tEVersion\n\n");
        }
        count = 0;
        fprintf(fsql,"\t\t) VALUES (\n");
        for(count==0;count<totRows;count++){
            strcpy(teststring,"VARBINARY");
            compresult = strncmp(types[count], teststring,9);
            tp=2;
            if(compresult==0){
                tp=6;
            }
            if(count==totRows-1){
                if(tp==6){
                    fprintf(fsql,"\t\t\tsetEDataHKDF(v%s,vKey,@iv,@salt),\n",fields[count]);
                }else{
                    fprintf(fsql,"\t\t\tv%s,\n",fields[count]);	
                }
            }else{
                if(tp==6){
                    fprintf(fsql,"\t\t\tsetEDataHKDF(v%s,vKey,@iv,@salt),\n",fields[count]);
                }else{
                    fprintf(fsql,"\t\t\tv%s,\n",fields[count]);	
                }
            }
        }
        fprintf(fsql,"\t\t\tvCurrentUserID \n");	
        if(hasBIN==1){
            fprintf(fsql,"\t\t,@iv,\n\n");
            fprintf(fsql,"\t\t@salt,\n\n");
            fprintf(fsql,"\t\t1\n\n");
        }
        fprintf(fsql,"\t\t); \n\t\tselect LAST_INSERT_ID() as %sID;\n\tEND IF;\nEND \n//\n\n",argv[1]);
        //// next create the GET procedure
        fprintf(fsql,"\n\nDROP PROCEDURE IF EXISTS sp_get%s;\n\n",argv[1]);
        fprintf(fsql,"CREATE PROCEDURE sp_get%s(\n",argv[1]);

        fprintf(fsql,"\tv%sID INT \n",argv[1]);
        fprintf(fsql,"\t)\nBEGIN \n");
        fprintf(fsql,"\tSelect * from tbl%s where %sID=v%sID AND RecordDeleted=0;\nEND\n\n",argv[1],argv[1],argv[1]);

        fprintf(fsql,"\n\n//\n\n");
        fprintf(fsql,"DROP PROCEDURE IF EXISTS sp_delete%s;\n\n", argv[1]);
        fprintf(fsql,"CREATE PROCEDURE sp_delete%s(\n	v%sID INT, vCurrentUserID INT\n)\n", argv[1],argv[1]);
        fprintf(fsql,"BEGIN\n\tUPDATE tbl%s SET RecordDeleted=1, ModifiedByUserID=vCurrentUserID WHERE %sID=v%sID;\nEND\n",argv[1],argv[1],argv[1]);

        fprintf(fsql,"\n\n//\n\n");
        
        fprintf(fsql,"DROP PROCEDURE IF EXISTS sp_Lock%sRecord;\n\n", argv[1]);
        fprintf(fsql,"CREATE PROCEDURE sp_Lock%sRecord(\n\tv%sID INT, vCurrentUserID INT\n)\n", argv[1],argv[1]);
        fprintf(fsql,"BEGIN\n\tUPDATE tbl%s SET RecordLockByUserID=vCurrentUserID, RecordLockTime=CURRENT_TIMESTAMP WHERE %sID=v%sID;\nEND\n",argv[1],argv[1],argv[1]);

        fprintf(fsql,"\n\n//\n\n");
        
        fprintf(fsql,"DROP PROCEDURE IF EXISTS sp_Unlock%sRecord;\n\n", argv[1]);
        fprintf(fsql,"CREATE PROCEDURE sp_Unlock%sRecord(\n	v%sID INT\n)\n", argv[1],argv[1]);
        fprintf(fsql,"BEGIN\n\tUPDATE tbl%s SET RecordLockByUserID=0, RecordLockTime=NULL WHERE %sID=v%sID;\nEND\n",argv[1],argv[1],argv[1]);

        fprintf(fsql,"\n\n//\n\n");
        fprintf(fsql,"\n/**************************************** \n");
        fprintf(fsql,"     end  %s \n",argv[1]);
        fprintf(fsql,"****************************************/ \n\n");

        fclose(fsql);

    }else{
        printf("could not open sql file");
    }


    //// step 3
    //// CREATE MS sql file

    char filenamesql[128];
    snprintf(filenamesql, sizeof filenamesql, "%s.sql",argv[1]);


    FILE *fmsql;
    fmsql = fopen(filenamesql,"w");

    count = 0;

    /// CREATE TABLE first
    if(fmsql!=NULL){
        fprintf(fmsql,"/**************************************** \n");
        fprintf(fmsql,"      %s \n",argv[1]);
        fprintf(fmsql,"****************************************/ \n\n");
        fprintf(fmsql,"GO\n");
        fprintf(fmsql,"IF OBJECT_ID(N'dbo.tbl%s', N'U') IS NULL \n",argv[1]);
        fprintf(fmsql,"BEGIN\n");
        fprintf(fmsql,"\tCREATE TABLE tbl%s(\n",argv[1]);
        fprintf(fmsql,"\t\t%sID\t\tINT PRIMARY KEY IDENTITY(1,1) NOT NULL,\n",argv[1]);
        for(count==0;count<totRows;count++){
            tp=2;
            strcpy(teststring,"DECIMAL(10,2)");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=3;
            }
            strcpy(teststring,"VARBINARY");
            compresult = strncmp(types[count], teststring,9);
            if(compresult==0){
                hasBIN=1;
                tp=6;
            }
            if(tp==3){
                fprintf(fmsql,"\t\t%s\t\tFLOAT %s,\n",fields[count],extra[count]);
            }else{
                fprintf(fmsql,"\t\t%s\t\t%s %s,\n",fields[count],types[count],extra[count]);
            }
        }
        if(hasBIN==1){
            //fprintf(fmsql,"\t\tEIV\tVARBINARY(255),\n");
            fprintf(fmsql,"\t\tES\tVARBINARY(16),\n");
            fprintf(fmsql,"\t\tEVersion\tINT,\n");
        }
        fprintf(fmsql,"\t\tRecordDeleted\t\tINT DEFAULT 0,\n");
        fprintf(fmsql,"\t\tRecordLockByUserID\tINT DEFAULT 0,\n");
        fprintf(fmsql,"\t\tRecordLockTime\t\tDATETIME NULL,\n");
        fprintf(fmsql,"\t\tdateCreated\t\tDATETIME DEFAULT CURRENT_TIMESTAMP,\n");
        fprintf(fmsql,"\t\tdateModified\t\tDATETIME DEFAULT CURRENT_TIMESTAMP,\n");
        fprintf(fmsql,"\t\tModifiedByUserID\tINT DEFAULT 0\n");
        //fprintf(fmsql,"\tPRIMARY KEY(%sID)\n",argv[1]);
        fprintf(fmsql,"\t)\n");
        fprintf(fmsql,"END;\n");
        fprintf(fmsql,"GO\n");
        

    /// now we create the procs
    //  sp_update proc
        count=0;
        fprintf(fmsql,"CREATE OR ALTER PROCEDURE sp_update%s\n",argv[1]);
        fprintf(fmsql,"\t@%sID\t\tINT,\n",argv[1]);
        compresult = 0;
        tp=0;
        for(count==0;count<totRows;count++){
            tp=2;
            strcpy(teststring,"VARBINARY");
            compresult = strncmp(types[count], teststring,9);
            if(compresult==0){
                tp=6;
            }
            strcpy(teststring,"DECIMAL(10,2)");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=3;
            }
            //if(count==totRows-1){
                //fprintf(fmsql,"	v%s	%s\n",fields[count],types[count]);
                // have added currentuser at the end so no need to look for comma

            //    fprintf(fmsql,"\t@%s\t%s,\n",fields[count],types[count]);
            //}else{
            if(tp==6){
                char newname[128];
                strcpy(newname,types[count]);
                replaceSubstr(newname, "VARBINARY", "VARCHAR");
                fprintf(fmsql,"\t@%s\t%s,\n",fields[count],newname);
            }else if(tp==3){
                fprintf(fmsql,"\t@%s\tFLOAT,\n",fields[count]);
            
            }else{
                fprintf(fmsql,"\t@%s\t%s,\n",fields[count],types[count]);
            }
            //}
        }
        fprintf(fmsql,"\t@CurrentUserID\tINT\n");
        //if(hasBIN==1){
        //    fprintf(fmsql,"\t,@Key\tVARCHAR(1000)\n");
        //}
        count = 0;
        fprintf(fmsql,"\n");
        fprintf(fmsql,"AS\n");
        fprintf(fmsql,"BEGIN\n");
        fprintf(fmsql,"\tDECLARE @count INT;\n\n");
        fprintf(fmsql,"\tSELECT @count=COUNT(%sID) FROM tbl%s where %sID=@%sID;\n",argv[1],argv[1],argv[1],argv[1]);
        fprintf(fmsql,"\tIF(@count > 0)\n");
        fprintf(fmsql,"\tBEGIN\n");
        fprintf(fmsql,"\t\tUPDATE tbl%s SET\n",argv[1]);
        compresult = 0;
        tp=0;
        for(count==0;count<totRows;count++){

            strcpy(teststring,"VARBINARY");
            compresult = strncmp(types[count], teststring,9);
            tp=2;
            if(compresult==0){
                tp=6;
            }
            if(count==totRows-1){
                //fprintf(fmsql,"		%s=v%s\n",fields[count],fields[count]);	
                if(tp==6){
                    fprintf(fmsql,"\t\t%s=CAST(dbo.setEData(@%s,ES) AS %s),\n",fields[count],fields[count],types[count]);	
                }else{
                    fprintf(fmsql,"\t\t%s=@%s,\n",fields[count],fields[count]);	
                }
            }else{
                if(tp==6){
                    fprintf(fmsql,"\t\t%s=CAST(dbo.setEData(@%s,ES) AS %s),\n",fields[count],fields[count], types[count]);
                }else{
                    fprintf(fmsql,"\t\t%s=@%s,\n",fields[count],fields[count]);
                }    
            }
        }
        fprintf(fmsql,"\t\tModifiedByUserID=@CurrentUserID,\n");	
        fprintf(fmsql,"\t\tdateModified=SYSDATETIME()\n");	
        fprintf(fmsql,"\t\tWHERE %sID = @%sID;\n",argv[1],argv[1]);
        fprintf(fmsql,"\t\tselect @%sID as %sID;\n",argv[1],argv[1]);

        count=0;	
        fprintf(fmsql,"\tEND\n\n");
        fprintf(fmsql,"\tELSE BEGIN\n\n");
        if(hasBIN==1){
            fprintf(fmsql,"\t\tDECLARE @salt VARBINARY(16);\n");
            fprintf(fmsql,"\t\tSET @salt=CRYPT_GEN_RANDOM(16);\n");
        }
        fprintf(fmsql,"\t\tINSERT INTO tbl%s(\n",argv[1]);
        for(count==0;count<totRows;count++){
            tp=2;
            strcpy(teststring,"VARBINARY");
            compresult = strncmp(types[count], teststring,9);
            if(compresult==0){
                tp=6;
            }
            if(count==totRows-1){
                //fprintf(fmsql,"		%s\n",fields[count]);	
                fprintf(fmsql,"\t\t\t%s,\n",fields[count]);	
            }else{
                fprintf(fmsql,"\t\t\t%s,\n",fields[count]);	
            }
        }
        fprintf(fmsql,"\t\t\tModifiedByUserID\n");	
        if(hasBIN==1){
            fprintf(fmsql,"\t\t\t,ES,\n");
            fprintf(fmsql,"\t\t\tEVersion\n");
        }
        count = 0;
        fprintf(fmsql,"\t\t) VALUES (\n");
        for(count==0;count<totRows;count++){
            tp=2;
            strcpy(teststring,"VARBINARY");
            compresult = strncmp(types[count], teststring,9);
            if(compresult==0){
                tp=6;
            }
            strcpy(teststring,"DECIMAL(10,2)");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=3;
            }
            if(tp==6){
                fprintf(fmsql,"\t\t\tCAST(dbo.setEData(@%s,@salt) AS %s),\n",fields[count], types[count]);
            }else if(tp==3){
                /// decimal
                fprintf(fmsql,"\t\t\t@%s,\n",fields[count]);
            }else{
                fprintf(fmsql,"\t\t\t@%s,\n",fields[count]);
            }
        }
        fprintf(fmsql,"\t\t\t@CurrentUserID \n");	
        if(hasBIN==1){
            fprintf(fmsql,"\t\t\t,@salt,\n");
            fprintf(fmsql,"\t\t\t1\n");
        }
        fprintf(fmsql,"\t\t); \n\t\tselect SCOPE_IDENTITY() as %sID;\n",argv[1]);
        fprintf(fmsql,"\t\tEND;\n");
        fprintf(fmsql,"\nEND; \n");
        fprintf(fmsql,"\nGO \n");
        //// next create the GET procedure
        fprintf(fmsql,"CREATE OR ALTER PROCEDURE sp_get%s\n",argv[1]);

        fprintf(fmsql,"\t@%sID INT \n",argv[1]);
        fprintf(fmsql,"\tAS \n");
        fprintf(fmsql,"\tBEGIN \n");
        fprintf(fmsql,"\tSelect * from tbl%s where %sID=@%sID AND RecordDeleted=0;\nEND\n\n",argv[1],argv[1],argv[1]);

        fprintf(fmsql,"\n\nGO\n\n");
        fprintf(fmsql,"CREATE OR ALTER PROCEDURE sp_delete%s\n	@%sID INT, @CurrentUserID INT\n\n", argv[1],argv[1]);
        fprintf(fmsql,"\tAS \n");
        fprintf(fmsql,"BEGIN\n\tUPDATE tbl%s SET RecordDeleted=1, ModifiedByUserID=@CurrentUserID WHERE %sID=@%sID;\nEND\n",argv[1],argv[1],argv[1]);

        fprintf(fmsql,"\n\nGO\n\n");
        
        fprintf(fmsql,"CREATE OR ALTER PROCEDURE sp_Lock%sRecord\n\t@%sID INT, @CurrentUserID INT\n\n", argv[1],argv[1]);
        fprintf(fmsql,"\tAS \n");

        fprintf(fmsql,"BEGIN\n\tUPDATE tbl%s SET RecordLockByUserID=@CurrentUserID, RecordLockTime=CURRENT_TIMESTAMP WHERE %sID=@%sID;\nEND\n",argv[1],argv[1],argv[1]);

        fprintf(fmsql,"\n\nGO\n\n");
        
        fprintf(fmsql,"CREATE OR ALTER PROCEDURE sp_Unlock%sRecord\n	@%sID INT\n", argv[1],argv[1]);
        fprintf(fmsql,"\tAS \n");
        fprintf(fmsql,"BEGIN\n\tUPDATE tbl%s SET RecordLockByUserID=0, RecordLockTime=NULL WHERE %sID=@%sID;\nEND\n",argv[1],argv[1],argv[1]);
        fprintf(fmsql,"\n\nGO\n\n");

        fprintf(fmsql,"\n/**************************************** \n");
        fprintf(fmsql,"     end  %s \n",argv[1]);
        fprintf(fmsql,"****************************************/ \n\n");

        fclose(fmsql);

    }else{
        printf("could not open sql file");
    }













    //// create the cs object class

    char filenamecs[128];
    snprintf(filenamecs, sizeof filenamecs, "%s.cs",argv[1]);


    FILE *fcs;
    fcs = fopen(filenamecs,"w");

    count = 0;

    if(fcs!=NULL){
        
        fprintf(fcs,"\n\npublic class %s{\n",argv[1]);

        fprintf(fcs,"\tprotected string _LastError = \"\";\n");
        fprintf(fcs,"\tprotected int _%sID;\n",argv[1]);
        count = 0;
        compresult = 0;
        tp = 2; // 0 = int, 1 = float, 2 = string, 3=decimal, 4=date
        for(count==0;count<totRows;count++){
            tp = 2; // 0 = int, 1 = float, 2 = string, 3=decimal, 4=date, 5=time, 6=varbinary
            strcpy(teststring,"INT");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            }        
            strcpy(teststring,"int");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            } 
            strcpy(teststring,"TINYINT");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            }        

            strcpy(teststring,"DECIMAL(10,2)");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=3;
            }
            strcpy(teststring,"DATE");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=4;
            }        
            strcpy(teststring,"DATETIME");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=4;
            }
            strcpy(teststring,"TIME");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=5;
            }
            
            strcpy(teststring,"VARBINARY");
            compresult = strncmp(types[count], teststring,9);
            if(compresult==0){
                tp=6;
            }
            

            if(tp==0){
                fprintf(fcs,"\tprotected int _%s;\n", fields[count]);
            }else if(tp==1){
                fprintf(fcs,"\tprotected float _%s;\n", fields[count]);
            }else if(tp==2){
                fprintf(fcs,"\tprotected string _%s = \"\";\n", fields[count]);
            }else if(tp==3){
                fprintf(fcs,"\tprotected double _%s;\n", fields[count]);
            }else if(tp==4){
                fprintf(fcs,"\tprotected DateTime _%s;\n", fields[count]);
            }else if(tp==5){
                fprintf(fcs,"\tprotected DateTime _%s;\n", fields[count]);
            }else if(tp==6){
                fprintf(fcs,"\tprotected string _%s = \"\";\n", fields[count]);
            }else{
                fprintf(fcs,"\tprotected int _%s;\n", fields[count]);
            }
        }
        fprintf(fcs,"\tprotected bool _RecordDeleted;\n");
        fprintf(fcs,"\tprotected int _RecordLockByUserID;\n");
        fprintf(fcs,"\tprotected DateTime _RecordLockTime;\n");
        fprintf(fcs,"\tprotected DateTime _dateCreated;\n");
        fprintf(fcs,"\tprotected DateTime _dateModified;\n");
        fprintf(fcs,"\tprotected int _ModifiedByUserID;\n");
        
        fprintf(fcs,"\n\tpublic int %sID {get=>_%sID; set=> _%sID=value;}\n",argv[1],argv[1],argv[1]);
        fprintf(fcs,"\n\tpublic string LastError {get=>_LastError; set=> _LastError=value;}\n");
        count = 0;
        compresult = 0;
        tp = 2; // 0 = int, 1 = float, 2 = string, 3=decimal, 4=date
        for(count==0;count<totRows;count++){
            tp = 2; // 0 = int, 1 = float, 2 = string, 3=decimal, 4=date
            strcpy(teststring,"INT");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            }        
            strcpy(teststring,"TINYINT");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            }        
            strcpy(teststring,"int");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            } 

            strcpy(teststring,"DECIMAL(10,2)");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=3;
            }
            strcpy(teststring,"DATE");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=4;
            }        
            strcpy(teststring,"DATETIME");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=4;
            }
            strcpy(teststring,"TIME");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=5;
            }

            if(tp==0){
                fprintf(fcs,"\tpublic int  %s {get=>_%s; set=> _%s=value;}\n", fields[count], fields[count], fields[count]);
            }else if(tp==1){
                fprintf(fcs,"\tpublic float %s {get=>_%s; set=> _%s=value;}\n", fields[count], fields[count], fields[count]);
            }else if(tp==2){
                fprintf(fcs,"\tpublic string %s {get=>_%s; set=> _%s=value;}\n", fields[count], fields[count], fields[count]);
                
            }else if(tp==3){
                fprintf(fcs,"\tpublic double %s {get=>_%s; set=> _%s=value;}\n", fields[count], fields[count], fields[count]);
            }else if(tp==4){
                fprintf(fcs,"\tpublic DateTime %s {get=>_%s; set=> _%s=value;}\n", fields[count], fields[count], fields[count]);
            }else if(tp==5){
                fprintf(fcs,"\tpublic DateTime %s {get=>_%s; set=> _%s=value;}\n", fields[count], fields[count], fields[count]);
            }else{
                fprintf(fcs,"\tpublic int %s {get=>_%s; set=> _%s=value;}\n", fields[count], fields[count], fields[count]);
            }
            
        }
        fprintf(fcs,"\tpublic bool RecordDeleted {get=>_RecordDeleted; set=> _RecordDeleted=value;}\n");
        fprintf(fcs,"\tpublic int RecordLockByUserID {get=>_RecordLockByUserID; set=> _RecordLockByUserID=value;}\n");
        fprintf(fcs,"\tpublic DateTime RecordLockTime {get=>_RecordLockTime; set=> _RecordLockTime=value;}\n");
        fprintf(fcs,"\tpublic DateTime dateCreated {get=>_dateCreated; set=> _dateCreated=value;}\n");
        fprintf(fcs,"\tpublic DateTime dateModified {get=>_dateModified; set=> _dateModified=value;}\n");
        fprintf(fcs,"\tpublic int ModifiedByUserID {get=>_ModifiedByUserID; set=> _ModifiedByUserID=value;}\n");

        fprintf(fcs,"}\n\n");
        
        fprintf(fcs,"public class %sD{\n",argv[1]);
        fprintf(fcs,"\tprotected string _LastErrorD = \"\";\n");
        fprintf(fcs,"\n\tpublic string LastErrorD {get=>_LastErrorD; set=> _LastErrorD=value;}\n\n");
        fprintf(fcs,"\tpublic %s load(int ID, %s o){\n",argv[1],argv[1]);
        fprintf(fcs,"\t\to.%sID = ID;\n",argv[1]);
        fprintf(fcs,"\t\tstring connString = DALGlobal.connectionString; \n");
        fprintf(fcs,"\t\tusing (MySqlConnection con = new MySqlConnection(connString)){\n");
        fprintf(fcs,"\t\t\tcon.Open();\n");
        fprintf(fcs,"\t\t\to = load(ID, con, o);\n");
        fprintf(fcs,"\t\t\tcon.Close();\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn o;\n");
        fprintf(fcs,"\t}\n");


        fprintf(fcs,"\tpublic %s load(int ID, MySqlConnection con, %s o", argv[1], argv[1]);
        if(hasBIN==1){
            fprintf(fcs,",string key");
        }
        fprintf(fcs,"){\n");
        fprintf(fcs,"\t\t_LastErrorD = \"\";\n");
        fprintf(fcs,"\t\to.%sID = ID;\n",argv[1]);
        fprintf(fcs,"\t\tstring sql = \"SELECT \";\n");
        fprintf(fcs,"\t\tsql += \"%sID \";\n", argv[1]);
        count = 0;
        for(count==0;count<totRows;count++){
            strcpy(teststring,"VARBINARY");
            compresult = strncmp(types[count], teststring,9);
            tp=2;
            if(compresult==0){
                tp=6;
            }
            if(tp==6){
                fprintf(fcs,"\t\tsql += \",getEDataHKDF(%s,'\" + key + \"',EIV,ES) as %s \";\n", fields[count],fields[count]);
            }else{
                fprintf(fcs,"\t\tsql += \",%s \";\n", fields[count]);
            }
        }
        fprintf(fcs,"\t\tsql += \",RecordDeleted \";\n");
        fprintf(fcs,"\t\tsql += \",RecordLockByUserID \";\n");
        fprintf(fcs,"\t\tsql += \",RecordLockTime \";\n");
        fprintf(fcs,"\t\tsql += \",dateCreated \";\n");
        fprintf(fcs,"\t\tsql += \",dateModified \";\n");
        fprintf(fcs,"\t\tsql += \",ModifiedByUserID \";\n");
        fprintf(fcs,"\t\tsql += \"FROM tbl%s  \";\n", argv[1]);
        fprintf(fcs,"\t\tsql += \"WHERE %sID=\" + o.%sID + \";\";\n", argv[1],argv[1]);
        fprintf(fcs,"\t\ttry{\n");
        fprintf(fcs,"\t\t\tusing(MySqlCommand cmd = new MySqlCommand(sql,con)){\n");
        fprintf(fcs,"\t\t\t\tMySqlDataReader r;\n");
        fprintf(fcs,"\t\t\t\tusing(r = cmd.ExecuteReader()){\n");
        fprintf(fcs,"\t\t\t\t\twhile(r.Read()){\n");
        count = 0;
        compresult = 0;
        tp = 2; // 0 = int, 1 = float, 2 = string, 3=decimal, 4=date
        for(count==0;count<totRows;count++){
            tp = 2; // 0 = int, 1 = float, 2 = string, 3=decimal, 4=date
            strcpy(teststring,"INT");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            }        
            strcpy(teststring,"TINYINT");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            }        
            strcpy(teststring,"int");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            } 

            strcpy(teststring,"DECIMAL(10,2)");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=3;
            }
            strcpy(teststring,"DATE");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=4;
            }        
            strcpy(teststring,"DATETIME");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=4;
            }
            strcpy(teststring,"TIME");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=5;
            }

            if(tp==0){
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? 0 : r.GetInt32(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else if(tp==1){
                // float
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? 0 : r.GetInt32(\"%s\");\n", fields[count], fields[count], fields[count]);
                
            }else if(tp==2){
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? \"\" : r.GetString(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else if(tp==3){
                // double
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? 0 : r.GetInt32(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else if(tp==4){
                // DateTime
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? DateTime.MinValue : r.GetDateTime(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else if(tp==5){
                // Time
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? DateTime.MinValue : r.GetDateTime(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else{
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? \"\" : r.GetString(\"%s\");\n", fields[count], fields[count], fields[count]);
            }
            
        }
        fprintf(fcs,"\t\t\t\t\t\to.RecordDeleted =r.IsDBNull(\"RecordDeleted\") ? false : r.GetBoolean(\"RecordDeleted\");\n");
        fprintf(fcs,"\t\t\t\t\t\to.RecordLockByUserID =r.IsDBNull(\"RecordLockByUserID\") ? 0 : r.GetInt32(\"RecordLockByUserID\");\n");
        fprintf(fcs,"\t\t\t\t\t\to.RecordLockTime =r.IsDBNull(\"RecordLockTime\") ? DateTime.MinValue : r.GetDateTime(\"RecordLockTime\");\n");
        fprintf(fcs,"\t\t\t\t\t\to.dateCreated =r.IsDBNull(\"dateCreated\") ? DateTime.MinValue : r.GetDateTime(\"dateCreated\");\n");
        fprintf(fcs,"\t\t\t\t\t\to.dateModified =r.IsDBNull(\"dateModified\") ? DateTime.MinValue : r.GetDateTime(\"dateModified\");\n");
        fprintf(fcs,"\t\t\t\t\t\to.ModifiedByUserID =r.IsDBNull(\"ModifiedByUserID\") ? 0 : r.GetInt32(\"ModifiedByUserID\");\n");


        fprintf(fcs,"\t\t\t\t\t}\n");
        fprintf(fcs,"\t\t\t\t\tr.Close();\n");
        fprintf(fcs,"\t\t\t\t}\n");
        fprintf(fcs,"\t\t\t}\n");
        fprintf(fcs,"\t\t\to.LastError = \"\";\n");
        fprintf(fcs,"\t\t\t_LastErrorD = \"\";\n");
//        fprintf(fcs,"\t\t\tret = true;\n");
        fprintf(fcs,"\t\t}catch(Exception ex){\n");
        fprintf(fcs,"\t\t\to.LastError = ex.Message;\n");
        fprintf(fcs,"\t\t\t_LastErrorD = ex.Message;\n");
  //      fprintf(fcs,"\t\t\tret = false;\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn o;\n");
        fprintf(fcs,"\t}\n");
        
        fprintf(fcs,"\tpublic %s save(%s o, int curUserID",argv[1],argv[1]);
        if(hasBIN==1){
            fprintf(fcs,", string key");
        }
        fprintf(fcs,"){\n");
        fprintf(fcs,"\t\t_LastErrorD = \"\";\n");
        fprintf(fcs,"\t\tint ret = 0;\n");
        fprintf(fcs,"\t\tstring connString = DALGlobal.connectionString; \n");
        fprintf(fcs,"\t\tusing (MySqlConnection con = new MySqlConnection(connString)){\n");
        fprintf(fcs,"\t\t\tcon.Open();\n");
        fprintf(fcs,"\t\t\to = save(con,o,curUserID);\n");
        fprintf(fcs,"\t\t\tcon.Close();\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn o;\n");
        fprintf(fcs,"\t}\n");
        fprintf(fcs,"\n");
        fprintf(fcs,"\tpublic %s save(MySqlConnection con, %s o, int curUserID",argv[1],argv[1]);
        if(hasBIN==1){
            fprintf(fcs,",string key");
        }
        fprintf(fcs,"){\n");
        fprintf(fcs,"\t\tint ret = 0;\n");
        fprintf(fcs,"\t\tusing(MySqlCommand cmd = new MySqlCommand(\"sp_Update%s\",con)){\n", argv[1]);
        fprintf(fcs,"\t\t\tcmd.CommandType = CommandType.StoredProcedure;\n");
        fprintf(fcs,"\t\t\tcmd.Parameters.AddWithValue(\"v%sID\",o.%sID);\n",argv[1],argv[1]);
        count = 0;
        for(count==0;count<totRows;count++){
            fprintf(fcs,"\t\t\tcmd.Parameters.AddWithValue(\"v%s\",o.%s);\n",fields[count],fields[count]);
        }
        fprintf(fcs,"\t\t\tcmd.Parameters.AddWithValue(\"vCurrentUserID\",curUserID);\n");
        if(hasBIN==1){
            fprintf(fcs,"\t\t\tcmd.Parameters.AddWithValue(\"vKey\",key);\n");
        }
        fprintf(fcs,"\t\t\ttry{\n");
        fprintf(fcs,"\t\t\t\tvar recid = cmd.ExecuteScalar();\n");
        fprintf(fcs,"\t\t\t\tint.TryParse(recid.ToString(), out ret);\n");
        fprintf(fcs,"\t\t\t}catch (Exception ex){\n");
        fprintf(fcs,"\t\t\t\to.LastError = ex.Message;\n");
        fprintf(fcs,"\t\t\t\t_LastErrorD = ex.Message;\n");
        fprintf(fcs,"\t\t\t\tret = -1;\n");
        fprintf(fcs,"\t\t\t}\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\n");

        fprintf(fcs,"\t\to.%sID=ret;;\n",argv[1]);
        fprintf(fcs,"\t\treturn o;\n");
        fprintf(fcs,"\t}\n");

        fprintf(fcs,"\tpublic int delete(%s o, int curUserID){\n",argv[1]);
        fprintf(fcs,"\t\tint ret = 0;\n");
        fprintf(fcs,"\t\tstring connString = DALGlobal.connectionString; \n");
        fprintf(fcs,"\t\tusing (MySqlConnection con = new MySqlConnection(connString)){\n");
        fprintf(fcs,"\t\t\tcon.Open();\n");
        fprintf(fcs,"\t\t\tret = delete(con,o, curUserID);\n");
        fprintf(fcs,"\t\t\tcon.Close();\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn ret;\n");
        fprintf(fcs,"\t}\n");

        fprintf(fcs,"\tpublic int delete(MySqlConnection con, %s o, int curUserID){\n",argv[1]);
        fprintf(fcs,"\t\tAuditLog a = new AuditLog(curUserID, \"%s\", \"%sID\",o.%sID, AuditLog.ActionType.Delete, o.%sID, 0);\n",argv[1],argv[1],argv[1],argv[1]);
        fprintf(fcs,"\t\t(new AuditLogD()).save(a);\n");
        fprintf(fcs,"\t\tint ret = 0;\n");
        fprintf(fcs,"\t\tusing (MySqlCommand cmd = new MySqlCommand(\"sp_Delete%s\", con)){\n", argv[1]);
        fprintf(fcs,"\t\t\tcmd.CommandType = CommandType.StoredProcedure;\n");
        fprintf(fcs,"\t\t\tcmd.Parameters.AddWithValue(\"v%sID\",o.%sID);\n",argv[1],argv[1]);
        fprintf(fcs,"\t\t\tcmd.Parameters.AddWithValue(\"vCurrentUserID\",curUserID);\n");
        fprintf(fcs,"\t\t\ttry{\n");
        fprintf(fcs,"\t\t\t\tvar recid = cmd.ExecuteNonQuery();\n");
        fprintf(fcs,"\t\t\t\tint.TryParse(recid.ToString(), out ret);\n");
        fprintf(fcs,"\t\t\t}catch (Exception ex){\n");
        fprintf(fcs,"\t\t\t\t_LastErrorD = ex.Message;\n");
        fprintf(fcs,"\t\t\t\tret = -1;\n");
        fprintf(fcs,"\t\t\t}\n");
        
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn ret;\n");
        fprintf(fcs,"\t}\n\n\n");
        
        fprintf(fcs,"\tpublic List<%s> get%ssL(){\n",argv[1],argv[1]);
        fprintf(fcs,"\t\tList<%s> l = new List<%s>();\n",argv[1],argv[1]);
        fprintf(fcs,"\t\tstring connString = DALGlobal.connectionString; \n");
        fprintf(fcs,"\t\tusing (MySqlConnection con = new MySqlConnection(connString)){\n");
        fprintf(fcs,"\t\t\tcon.Open();\n");
        fprintf(fcs,"\t\t\tl = get%ssL(con);\n",argv[1]);
        fprintf(fcs,"\t\t\tcon.Close();\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn l;\n");
        fprintf(fcs,"\t}\n");
                    
        fprintf(fcs,"\tpublic List<%s> get%ssL(MySqlConnection con){\n", argv[1], argv[1]);

        fprintf(fcs,"\t\tList<%s> l = new List<%s>();\n",argv[1],argv[1]);
        fprintf(fcs,"\t\t_LastErrorD = \"\";\n");
        fprintf(fcs,"\t\tstring sql = \"SELECT \";\n");
        fprintf(fcs,"\t\tsql += \"%sID \";\n", argv[1]);
        count = 0;
        for(count==0;count<totRows;count++){
            strcpy(teststring,"VARBINARY");
            compresult = strncmp(types[count], teststring,9);
            tp=2;
            if(compresult==0){
                tp=6;
            }
            if(tp==6){
                fprintf(fcs,"\t\tsql += \",getEDataHKDF(%s,'\" + key + \"',EIV,ES) as %s \";\n", fields[count],fields[count]);
            }else{
                fprintf(fcs,"\t\tsql += \",%s \";\n", fields[count]);
            }
        }
        fprintf(fcs,"\t\tsql += \",RecordDeleted \";\n");
        fprintf(fcs,"\t\tsql += \",RecordLockByUserID \";\n");
        fprintf(fcs,"\t\tsql += \",RecordLockTime \";\n");
        fprintf(fcs,"\t\tsql += \",dateCreated \";\n");
        fprintf(fcs,"\t\tsql += \",dateModified \";\n");
        fprintf(fcs,"\t\tsql += \",ModifiedByUserID \";\n");
        fprintf(fcs,"\t\tsql += \" FROM tbl%s  \";\n", argv[1]);
        fprintf(fcs,"\t\tsql += \" WHERE RecordDeleted=0  \";\n");
        fprintf(fcs,"\t\t//sql += \"AND %sID=\" + o.%sID + \";\";\n", argv[1],argv[1]);
        fprintf(fcs,"\t\ttry{\n");
        fprintf(fcs,"\t\t\tusing(MySqlCommand cmd = new MySqlCommand(sql,con)){\n");
        fprintf(fcs,"\t\t\t\tMySqlDataReader r;\n");
        fprintf(fcs,"\t\t\t\tusing(r = cmd.ExecuteReader()){\n");
        fprintf(fcs,"\t\t\t\t\twhile(r.Read()){\n");
        fprintf(fcs,"\t\t\t\t\t\t%s o = new %s();\n",argv[1],argv[1]);
        fprintf(fcs,"\t\t\t\t\t\to.%sID =r.IsDBNull(\"%sID\") ? 0 : r.GetInt32(\"%sID\");\n",argv[1],argv[1],argv[1]);
        count = 0;
        compresult = 0;
        tp = 2; // 0 = int, 1 = float, 2 = string, 3=decimal, 4=date
        for(count==0;count<totRows;count++){
            tp = 2; // 0 = int, 1 = float, 2 = string, 3=decimal, 4=date
            strcpy(teststring,"INT");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            }        
            strcpy(teststring,"TINYINT");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            }        
            strcpy(teststring,"int");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            } 

            strcpy(teststring,"DECIMAL(10,2)");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=3;
            }
            strcpy(teststring,"DATE");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=4;
            }        
            strcpy(teststring,"DATETIME");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=4;
            }
            strcpy(teststring,"TIME");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=5;
            }

            if(tp==0){
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? 0 : r.GetInt32(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else if(tp==1){
                // float
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? 0 : r.GetInt32(\"%s\");\n", fields[count], fields[count], fields[count]);
                
            }else if(tp==2){
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? \"\" : r.GetString(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else if(tp==3){
                // double
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? 0 : r.GetInt32(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else if(tp==4){
                // DateTime
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? DateTime.MinValue : r.GetDateTime(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else if(tp==5){
                // Time
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? DateTime.MinValue : r.GetDateTime(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else{
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? \"\" : r.GetString(\"%s\");\n", fields[count], fields[count], fields[count]);
            }
            
        }
        fprintf(fcs,"\t\t\t\t\t\to.RecordDeleted =r.IsDBNull(\"RecordDeleted\") ? false : r.GetBoolean(\"RecordDeleted\");\n");
        fprintf(fcs,"\t\t\t\t\t\to.RecordLockByUserID =r.IsDBNull(\"RecordLockByUserID\") ? 0 : r.GetInt32(\"RecordLockByUserID\");\n");
        fprintf(fcs,"\t\t\t\t\t\to.RecordLockTime =r.IsDBNull(\"RecordLockTime\") ? DateTime.MinValue : r.GetDateTime(\"RecordLockTime\");\n");
        fprintf(fcs,"\t\t\t\t\t\to.dateCreated =r.IsDBNull(\"dateCreated\") ? DateTime.MinValue : r.GetDateTime(\"dateCreated\");\n");
        fprintf(fcs,"\t\t\t\t\t\to.dateModified =r.IsDBNull(\"dateModified\") ? DateTime.MinValue : r.GetDateTime(\"dateModified\");\n");
        fprintf(fcs,"\t\t\t\t\t\to.ModifiedByUserID =r.IsDBNull(\"ModifiedByUserID\") ? 0 : r.GetInt32(\"ModifiedByUserID\");\n");


        fprintf(fcs,"\t\t\t\t\t\tl.Add(o);\n");
        fprintf(fcs,"\t\t\t\t\t}\n");
        fprintf(fcs,"\t\t\t\t\tr.Close();\n");
        fprintf(fcs,"\t\t\t\t}\n");
        fprintf(fcs,"\t\t\t}\n");
        fprintf(fcs,"\t\t\t_LastErrorD = \"\";\n");
        fprintf(fcs,"\t\t}catch(Exception ex){\n");
        fprintf(fcs,"\t\t\t_LastErrorD = ex.Message;\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn l;\n");
        fprintf(fcs,"\t}\n");
       



        fprintf(fcs,"\tpublic DataTable get%ssDT(string srchString, List<UserViewField> fields,string SortBy, string SortDirection",argv[1]);
        if(hasBIN==1){
            fprintf(fcs,", string key");
        }
        fprintf(fcs,"){\n");
        fprintf(fcs,"\t\tDataTable dt = new DataTable();\n");
        fprintf(fcs,"\t\tstring connString = DALGlobal.connectionString; \n");
        fprintf(fcs,"\t\tusing (MySqlConnection con = new MySqlConnection(connString)){\n");
        fprintf(fcs,"\t\t\tcon.Open();\n");
        fprintf(fcs,"\t\t\tdt = get%ssDT(con,srchString, fields, SortBy, SortDirection",argv[1]);
        if(hasBIN==1){
            fprintf(fcs,",key");
        }
        fprintf(fcs,");\n");
        fprintf(fcs,"\t\t\tcon.Close();\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn dt;\n");
        fprintf(fcs,"\t}\n");
                    
        fprintf(fcs,"\tprivate DataTable get%ssDT(MySqlConnection con,string srchString, List<UserViewField> fields, string SortBy, string SortDirection", argv[1]);
        if(hasBIN==1){
            fprintf(fcs,", string key");
        }
        fprintf(fcs,"){\n");
        
        fprintf(fcs,"\t\tDataTable dt = new DataTable();\n");
        fprintf(fcs,"\t\t_LastErrorD = \"\";\n");
        fprintf(fcs,"\t\tstring sql = \"SELECT \";\n");
        fprintf(fcs,"\t\tstring strFields = \"\";\n");
        fprintf(fcs,"\t\tstring strIncKey = \"\";\n");
        fprintf(fcs,"\t\tstring strExtraJoins = \"\";\n");

        fprintf(fcs,"\t\tif(fields.Count>0){\n");
        fprintf(fcs,"\t\t\tforeach(UserViewField f in fields){\n");
        fprintf(fcs,"\t\t\t\tif(f.FieldName==\"tmpGUID\"){\n");
        fprintf(fcs,"\t\t\t\t\tstrFields += \",(SELECT UUID()) as tmpGUID \";\n");
        fprintf(fcs,"\t\t\t\t}\n");
        count = 0;
        int posss = 0;
        for(count==0;count<totRows;count++){
            strcpy(teststring,"VARBINARY");
            compresult = strncmp(types[count], teststring,9);
            tp=2;
            if(compresult==0){
                tp=6;
                fprintf(fcs,"\t\t\t\telse if(f.FieldName==\"%s\"){\n",fields[count]);
                fprintf(fcs,"\t\t\t\t\tstrFields += \", getEDataHKDF(%s,'\" + key + \"',EIV,ES) as %s\";\n",fields[count],fields[count]);
                if(hasBIN==1){
                    fprintf(fcs,"\t\t\t\t\tif(strIncKey.Length>0 && key.Length>0){\n");
                    fprintf(fcs,"\t\t\t\t\t\tstrIncKey += \" @key:='\" + key + \"' as kkkey,\";\n");
                    fprintf(fcs,"\t\t\t\t\t}\n");
                }
                fprintf(fcs,"\t\t\t\t}\n");
                posss ++;
            }
        }
        //fprintf(fcs,"\t\t\t\tif(f.FieldName==\"encryptedfieldexample\"){\n");
        //fprintf(fcs,"\t\t\t\t\tstrFields += \", getEDataHKDF(blah,@key,EIV,IS) as encryptedfieldexample \"\n");

        fprintf(fcs,"\t\t\t\telse{\n");
        fprintf(fcs,"\t\t\t\t\tstrFields += \", \" + f.FieldName + \" \";\n");
        fprintf(fcs,"\t\t\t\t\t//extraJoins += \" LEFT JOIN tblOtherTable as t on t.XID=x.XID \"\n");
        //if(posss>0){
            fprintf(fcs,"\t\t\t\t}\n");
        //}
        fprintf(fcs,"\t\t\t}\n");
        fprintf(fcs,"\t\t}else{\n");
        count = 0;
        for(count==0;count<totRows;count++){
            strcpy(teststring,"VARBINARY");
            compresult = strncmp(types[count], teststring,9);
            tp=2;
            if(compresult==0){
                tp=6;
            }
            if(count==0){
                /// leave off the comma
                if(tp==6){
                    fprintf(fcs,"\t\t\tstrFields += \"setEDataHKDF(%s,'\" + key + \"',EIV,ES) as %s \";\n", fields[count],fields[count]);
                }else{
                    fprintf(fcs,"\t\t\tstrFields += \"%s \";\n", fields[count]);
                }
            }else{
                if(tp==6){
                    fprintf(fcs,"\t\t\tstrFields += \",setEDataHKDF(%s,'\" + key + \"',EIV,ES) as %s \";\n", fields[count], fields[count]);
                }else{
                    fprintf(fcs,"\t\t\tstrFields += \",%s \";\n", fields[count]);
                }
            }
        }
        fprintf(fcs,"\t\t\tstrFields += \",RecordDeleted \";\n");
        fprintf(fcs,"\t\t\tstrFields += \",RecordLockByUserID \";\n");
        fprintf(fcs,"\t\t\tstrFields += \",RecordLockTime \";\n");
        fprintf(fcs,"\t\t\tstrFields += \",dateCreated \";\n");
        fprintf(fcs,"\t\t\tstrFields += \",dateModified \";\n");
        fprintf(fcs,"\t\t\tstrFields += \",ModifiedByUserID \";\n");
        fprintf(fcs,"\t\t}\n");


        fprintf(fcs,"\t\tstring sortString = \"\";\n");
        fprintf(fcs,"\t\tif(SortBy.Length>0){\n");
            count = 0;
            for(count==0;count<totRows;count++){
                strcpy(teststring,"VARBINARY");
                compresult = strncmp(types[count], teststring,9);
                tp=2;
                if(compresult==0){
                    tp=6;
                    if(posss==0){
                        fprintf(fcs,"\t\t\t\tif(SortBy==\"%s\"){\n",fields[count]);
                    }else{
                        fprintf(fcs,"\t\t\t\telse if(SortyBy==\"%s\"){\n",fields[count]);
                    }
                    fprintf(fcs,"\t\t\t\t\tSortBy = \"getEDataHKDF(%s,'\" + key + \"',EIV,ES) \";\n",fields[count]);
                    if(hasBIN==1){
                        fprintf(fcs,"\t\t\t\t\tif(strIncKey.Length>0 && key.Length>0){\n");
                        fprintf(fcs,"\t\t\t\t\t\tstrIncKey += \" @key:='\" + key + \"' as kkkey,\";\n");
                        fprintf(fcs,"\t\t\t\t\t}\n");
                    }
                    fprintf(fcs,"\t\t\t\t}\n");
                    posss ++;
                }
            }
        fprintf(fcs,"\t\t\tsortString += \" ORDER BY \" + SortBy + \" \" + SortDirection + \" \";\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\tsql += strIncKey + \"%sID \" + strFields + \" \";\n", argv[1]);
        fprintf(fcs,"\t\tsql += \" FROM tbl%s  \";\n", argv[1]);
        fprintf(fcs,"\t\tsql += \" \" + strExtraJoins + \" \";\n");
        fprintf(fcs,"\t\tsql += \" WHERE tbl%s.RecordDeleted=0  \";\n",argv[1]);
        
        fprintf(fcs,"\t\tif(srchString.Length>0){\n");
        fprintf(fcs,"\t\t\tsql += \" AND Description LIKE '%%\" + srchString + \"%%'\";\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\tsql += \" \" + sortString +  \" \";\n");

        fprintf(fcs,"\t\ttry{\n");
        fprintf(fcs,"\t\t\tusing(MySqlDataAdapter da = new MySqlDataAdapter(sql,con)){\n");
        fprintf(fcs,"\t\t\t\tdt = new DataTable();\n");
        fprintf(fcs,"\t\t\t\tda.Fill(dt);\n");
        fprintf(fcs,"\t\t\t\t_LastErrorD = \"\";\n");
        fprintf(fcs,"\t\t\t}\n");
        fprintf(fcs,"\t\t}catch(Exception ex){\n");
        fprintf(fcs,"\t\t\t_LastErrorD = ex.Message;\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn dt;\n");
        fprintf(fcs,"\t}\n");

        /*fprintf(fcs,"\tpublic DataTable get%ssDA(string srchString, string strSelect, out MySqlDataAdapter da){\n",argv[1]);
        fprintf(fcs,"\t\tda = new MySqlDataAdapter();\n");
        fprintf(fcs,"\t\tDataTable dt = new DataTable();\n");
        fprintf(fcs,"\t\tstring connString = DALGlobal.connectionString; \n");
        fprintf(fcs,"\t\tusing (MySqlConnection con = new MySqlConnection(connString)){\n");
        fprintf(fcs,"\t\t\tcon.Open();\n");
        fprintf(fcs,"\t\t\tdt = get%ssDA(con,srchString, strSelect, out da);\n",argv[1]);
        fprintf(fcs,"\t\t\tcon.Close();\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn dt;\n");
        fprintf(fcs,"\t}\n");
                    
        fprintf(fcs,"\tprivate DataTable get%ssDA(MySqlConnection con,string srchString, string strSelect, out MySqlDataAdapter da){\n", argv[1]);

        fprintf(fcs,"\t\tda = new MySqlDataAdapter();\n");
        fprintf(fcs,"\t\tDataTable dt = new DataTable();\n");
        fprintf(fcs,"\t\t_LastErrorD = \"\";\n");
        fprintf(fcs,"\t\tstring sql = \"SELECT \";\n");
        fprintf(fcs,"\t\tsql += \"%sID, \";\n", argv[1]);
        fprintf(fcs,"\t\tif(strSelect.Length>0){\n");
        fprintf(fcs,"\t\t\tsql += strSelect;\n");
        fprintf(fcs,"\t\t}else{\n");
        count = 0;
        for(count==0;count<totRows;count++){
            if(count==0){
                /// leave off the comma
                fprintf(fcs,"\t\t\tsql += \"%s \";\n", fields[count]);
            }else{
                fprintf(fcs,"\t\t\tsql += \",%s \";\n", fields[count]);
            }
        }
        fprintf(fcs,"\t\t\tsql += \",RecordDeleted \";\n");
        fprintf(fcs,"\t\t\tsql += \",RecordLockByUserID \";\n");
        fprintf(fcs,"\t\t\tsql += \",RecordLockTime \";\n");
        fprintf(fcs,"\t\t\tsql += \",dateCreated \";\n");
        fprintf(fcs,"\t\t\tsql += \",dateModified \";\n");
        fprintf(fcs,"\t\t\tsql += \",ModifiedByUserID \";\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\tsql += \"FROM tbl%s  \";\n", argv[1]);
        fprintf(fcs,"\t\tif(srchString.Length>0){\n");
        fprintf(fcs,"\t\t\tsql += \" WHERE Description LIKE '%%\" + srchString + \"%%'\";\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\t//sql += \" ORDER BY Description \"\n");
        fprintf(fcs,"\t\ttry{\n");
        fprintf(fcs,"\t\t\tda = new MySqlDataAdapter(sql,con);\n");
        fprintf(fcs,"\t\t\tdt = new DataTable();\n");
        fprintf(fcs,"\t\t\tda.Fill(dt);\n");
        fprintf(fcs,"\t\t\t_LastErrorD = \"\";\n");
        fprintf(fcs,"\t\t}catch(Exception ex){\n");
        fprintf(fcs,"\t\t\t_LastErrorD = ex.Message;\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn dt;\n");
        fprintf(fcs,"\t}\n");
        */




        fprintf(fcs,"}\n\n\n");





        fprintf(fcs,"public class %sBL{\n",argv[1]);
        
        fprintf(fcs,"\tprotected string _LastErrorB = \"\";\n");
        fprintf(fcs,"\n\tpublic string LastErrorB {get=>_LastErrorB; set=> _LastErrorB=value;}\n\n");

        fprintf(fcs,"\tpublic %s load(int ID,%s o",argv[1],argv[1]);
        if(hasBIN==1){
            fprintf(fcs,", string key");
        }
        fprintf(fcs,"){\n");
        fprintf(fcs,"\t\to.%sID=ID; \n",argv[1]);
        fprintf(fcs,"\t\t%sD d = new %sD(); \n",argv[1],argv[1]);
        fprintf(fcs,"\t\to = d.load(ID,o");
        if(hasBIN==1){
            fprintf(fcs,",key");
        }
        fprintf(fcs,");\n");
        fprintf(fcs,"\t\t_LastErrorB = d.LastErrorD; \n");
        fprintf(fcs,"\t\treturn o; \n");

        fprintf(fcs,"\t}\n\n");

        fprintf(fcs,"\tpublic %s save(%s o,int curUserID",argv[1],argv[1]);
        if(hasBIN==1){
            fprintf(fcs,", string key");
        }
        fprintf(fcs,"){\n");
        fprintf(fcs,"\t\t_LastErrorB = \"\";\n");
        fprintf(fcs,"\t\t%sD d = new %sD(); \n",argv[1],argv[1]);
        fprintf(fcs,"\t\to = d.save(o,curUserID");
        if(hasBIN==1){
            fprintf(fcs,", string key");
        }
        fprintf(fcs,");\n");
        fprintf(fcs,"\t\t_LastErrorB = d.LastErrorD; \n");
        fprintf(fcs,"\t\treturn o; \n");
        fprintf(fcs,"\t}\n\n");
        
        fprintf(fcs,"\tpublic int delete(%s o,int curUserID){\n",argv[1]);
        fprintf(fcs,"\t\t_LastErrorB = \"\";\n");
        fprintf(fcs,"\t\t%sD d = new %sD(); \n",argv[1],argv[1]);
        fprintf(fcs,"\t\tint ret = d.delete(o,curUserID); \n");
        fprintf(fcs,"\t\t_LastErrorB = d.LastErrorD; \n");
        fprintf(fcs,"\t\treturn ret; \n");
        fprintf(fcs,"\t}\n\n");

        fprintf(fcs,"\tpublic void setHistory(%s nObj,%s oObj,int curUserID){\n",argv[1],argv[1]);
        fprintf(fcs,"\t\tAuditLogController a = new AuditLogController();\n");
        fprintf(fcs,"\t\ta.DoHistory(nObj, oObj, nObj.%sID,curUserID);\n",argv[1]);
        fprintf(fcs,"\t}\n\n");
        fprintf(fcs,"\tpublic void setHistory(%s o, int curUserID){\n",argv[1]);
        fprintf(fcs,"\t\tAuditLog a = new AuditLog(curUserID, \"%s\", \"%sID\",o.%sID, AuditLog.ActionType.Insert, o.%sID, 0);\n",argv[1],argv[1],argv[1],argv[1]);
        fprintf(fcs,"\t\t(new AuditLogBL()).save(a);\n");
        fprintf(fcs,"\t}\n");
        //fprintf(fcs,"}\n");

        fprintf(fcs,"\tpublic List<%s> get%ssL(){\n",argv[1],argv[1]);
        fprintf(fcs,"\t\t_LastErrorB = \"\";\n");
        fprintf(fcs,"\t\tList<%s> l = new List<%s>();\n",argv[1],argv[1]);
        fprintf(fcs,"\t\t%sD d = new %sD(); \n",argv[1],argv[1]);
        fprintf(fcs,"\t\tl = d.get%ssL(); \n",argv[1]);
        fprintf(fcs,"\t\t_LastErrorB = d.LastErrorD; \n");
        fprintf(fcs,"\t\treturn l; \n");
        fprintf(fcs,"\t}\n\n");
        
        /*
        fprintf(fcs,"\tpublic DataTable get%ssDT(string srchString,string strSelect,string SortBy, string SortDirection",argv[1]);
        if(hasBIN==1){
            fprintf(fcs,",string key");
        }
        fprintf(fcs,"){\n");
        fprintf(fcs,"\t\t_LastErrorB = \"\";\n");
        fprintf(fcs,"\t\tDataTable dt = new DataTable();\n");
        fprintf(fcs,"\t\t%sD d = new %sD(); \n",argv[1],argv[1]);
        fprintf(fcs,"\t\tdt = d.get%ssDT(srchString, strSelect, SortBy, SortDirection",argv[1]);
        if(hasBIN==1){
            fprintf(fcs,", key");
        }
        fprintf(fcs,");\n");
        fprintf(fcs,"\t\t_LastErrorB = d.LastErrorD; \n");
        fprintf(fcs,"\t\treturn dt; \n");
        fprintf(fcs,"\t}\n\n");
        */
        fprintf(fcs,"\tpublic DataTable get%ssDT(string srchString,int UserViewID, string SortBy, string SortDirection",argv[1]);
        if(hasBIN==1){
            fprintf(fcs,",string key");
        }
        fprintf(fcs,"){\n");
        fprintf(fcs,"\t\t_LastErrorB = \"\";\n");
        fprintf(fcs,"\t\tDataTable dt = new DataTable();\n");
        fprintf(fcs,"\t\t%sD d = new %sD(); \n",argv[1],argv[1]);
        fprintf(fcs,"\t\tif(UserViewID>0){\n");
        fprintf(fcs,"\t\t\tUserViewFieldBL uvbl = new UserViewFieldBL();; \n");
        fprintf(fcs,"\t\t\tList<UserViewField> o = new List<UserViewField>();\n");
        fprintf(fcs,"\t\t\to = uvbl.getUserViewFields(UserViewID);\n");
        /*fprintf(fcs,"\t\t\tif(o.Count > 0){\n");
        if(hasBIN==1){
            fprintf(fcs,"\t\t\t\tstring s = \"\";\n");
            fprintf(fcs,"\t\t\t\tstring m = \"\";\n");
            fprintf(fcs,"\t\t\t\tforeach(UserViewField f in o){\n");
            count = 0;
            int posss = 0;
            for(count==0;count<totRows;count++){
                strcpy(teststring,"VARBINARY");
                compresult = strncmp(types[count], teststring,9);
                tp=2;
                if(compresult==0){
                    tp=6;
                    if(posss==0){
                        fprintf(fcs,"\t\t\t\t\tif(f.FieldName==\"%s\"){\n",fields[count]);
                    }else{
                        fprintf(fcs,"\t\t\t\t\telse if(f.FieldName==\"%s\"){\n",fields[count]);
                    }
                    fprintf(fcs,"\t\t\t\t\t\ts += m + \"getEDataHKDF(%s,'\" + key + \"',EIV,ES) as %s\";\n",fields[count],fields[count]);
                    fprintf(fcs,"\t\t\t\t\t}\n");
                    posss ++;
                }
            }
            fprintf(fcs,"\t\t\t\t\telse{\n \t\t\t\t\t\ts += m + f.FieldName;\t\t\t\t\t}\n");

            fprintf(fcs,"\t\t\t\t\t\n");
            fprintf(fcs,"\t\t\t\t}\n");

        }else{
            fprintf(fcs,"\t\t\t\tstring s = string.Join(\",\",o.Select(x => BLLGlobal.SafeFieldName(x.FieldName)));\n");
        }
        fprintf(fcs,"\t\t\t\tdt = d.get%ssDT(srchString, s,SortBy, SortDirection); \n",argv[1]);
        */

        fprintf(fcs,"\t\t\tdt = d.get%ssDT(srchString,o,SortBy, SortDirection); \n",argv[1]);
        fprintf(fcs,"\t\t\t_LastErrorB = d.LastErrorD; \n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn dt; \n");
        fprintf(fcs,"\t}\n\n");
        
        fprintf(fcs,"\tpublic DataTable get%ssDT(string srchString,List<UserViewField> fields,string SortBy, string SortDirection){\n",argv[1]);
        fprintf(fcs,"\t\t_LastErrorB = \"\";\n");
        fprintf(fcs,"\t\tDataTable dt = new DataTable();\n");
        fprintf(fcs,"\t\t%sD d = new %sD(); \n",argv[1],argv[1]);
        /*fprintf(fcs,"\t\tif(fields.Count > 0){\n");
        if(hasBIN==1){
            fprintf(fcs,"\t\t\t\tstring s = \"\";\n");
            fprintf(fcs,"\t\t\t\tstring m = \"\";\n");
            fprintf(fcs,"\t\t\t\tforeach(UserViewField f in fields){\n");
            count = 0;
            int posss = 0;
            for(count==0;count<totRows;count++){
                strcpy(teststring,"VARBINARY");
                compresult = strncmp(types[count], teststring,9);
                tp=2;
                if(compresult==0){
                    tp=6;
                    if(posss==0){
                        fprintf(fcs,"\t\t\t\t\tif(f.FieldName==\"%s\"){\n",fields[count]);
                    }else{
                        fprintf(fcs,"\t\t\t\t\telse if(f.FieldName==\"%s\"){\n",fields[count]);
                    }
                    fprintf(fcs,"\t\t\t\t\t\ts += m + \"getEDataHKDF(%s,'\" + key + \"',EIV,ES) as %s\";\n",fields[count],fields[count]);
                    fprintf(fcs,"\t\t\t\t\t}\n");
                    posss ++;
                }
            }
            fprintf(fcs,"\t\t\t\t\telse{\n \t\t\t\t\t\ts += m + f.FieldName;\t\t\t\t\t}\n");

            fprintf(fcs,"\t\t\t\t\t\n");
            fprintf(fcs,"\t\t\t\t}\n");

        }else{
            fprintf(fcs,"\t\t\tstring s = string.Join(\",\",fields.Select(x => BLLGlobal.SafeFieldName(x.FieldName)));\n");
        }
        fprintf(fcs,"\t\t\tdt = d.get%ssDT(srchString, s,SortBy,SortDirection); \n",argv[1]);
        */
        fprintf(fcs,"\t\tdt = d.get%ssDT(srchString,fields,SortBy,SortDirection); \n",argv[1]);
        fprintf(fcs,"\t\t_LastErrorB = d.LastErrorD; \n");
        fprintf(fcs,"\t\treturn dt; \n");
        fprintf(fcs,"\t}\n\n");


        /*
        fprintf(fcs,"\tpublic DataTable get%ssDA(string srchString,string strSelect, out MySqlDataAdapter da){\n",argv[1]);
        fprintf(fcs,"\t\t_LastErrorB = \"\";\n");
        fprintf(fcs,"\t\tda = new MySqlDataAdapter();\n");
        fprintf(fcs,"\t\tDataTable dt = new DataTable();\n");
        fprintf(fcs,"\t\t%sD d = new %sD(); \n",argv[1],argv[1]);
        fprintf(fcs,"\t\tdt = d.get%ssDA(srchString, strSelect, out da); \n",argv[1]);
        fprintf(fcs,"\t\t_LastErrorB = d.LastErrorD; \n");
        fprintf(fcs,"\t\treturn dt; \n");
        fprintf(fcs,"\t}\n\n");

        fprintf(fcs,"\tpublic DataTable get%ssDA(string srchString,int UserViewID, out MySqlDataAdapter da){\n",argv[1]);
        fprintf(fcs,"\t\t_LastErrorB = \"\";\n");
        fprintf(fcs,"\t\tda = new MySqlDataAdapter();\n");
        fprintf(fcs,"\t\tDataTable dt = new DataTable();\n");
        fprintf(fcs,"\t\t%sD d = new %sD(); \n",argv[1],argv[1]);
        fprintf(fcs,"\t\tif(UserViewID>0){\n");
        fprintf(fcs,"\t\t\tUserViewFieldBL uvbl = new UserViewFieldBL();; \n");
        fprintf(fcs,"\t\t\tList<UserViewField> o = new List<UserViewField>();\n");
        fprintf(fcs,"\t\t\to = uvbl.getUserViewFields(UserViewID);\n");
        fprintf(fcs,"\t\t\tif(o.Count > 0){\n");
        fprintf(fcs,"\t\t\t\tstring s = string.Join(\",\",o.Select(x => BLLGlobal.SafeFieldName(x.FieldName)));\n");
        fprintf(fcs,"\t\t\t\tdt = d.get%ssDA(srchString, s, out da); \n",argv[1]);
        fprintf(fcs,"\t\t\t_LastErrorB = d.LastErrorD; \n");
        fprintf(fcs,"\t\t\t}else{\n");
        fprintf(fcs,"\t\t\t_LastErrorB = \"No Fields in this UserView\";\n");
        fprintf(fcs,"\t\t\t}\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn dt; \n");
        fprintf(fcs,"\t}\n\n");
        
        fprintf(fcs,"\tpublic DataTable get%ssDA(string srchString,List<UserViewField> fields,out MySqlDataAdapter da){\n",argv[1]);
        fprintf(fcs,"\t\t_LastErrorB = \"\";\n");
        fprintf(fcs,"\t\tDataTable dt = new DataTable();\n");
        fprintf(fcs,"\t\tda = new MySqlDataAdapter();\n");
        fprintf(fcs,"\t\t%sD d = new %sD(); \n",argv[1],argv[1]);
        fprintf(fcs,"\t\tif(fields.Count > 0){\n");
        fprintf(fcs,"\t\t\tstring s = string.Join(\",\",fields.Select(x => BLLGlobal.SafeFieldName(x.FieldName)));\n");
        fprintf(fcs,"\t\t\tdt = d.get%ssDA(srchString, s, out da); \n",argv[1]);
        fprintf(fcs,"\t\t\t_LastErrorB = d.LastErrorD; \n");
        fprintf(fcs,"\t\t}else{\n");
        fprintf(fcs,"\t\t\t_LastErrorB = \"No Fields in this UserView\";\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn dt; \n");
        fprintf(fcs,"\t}\n\n");
        */
















        fprintf(fcs,"//// put this into AuditLog.cs\n");
        fprintf(fcs,"\tpublic void DoHistory(%s oNew, %s oOld, int ObjectID,int curUserID){\n",argv[1],argv[1]);
        fprintf(fcs,"\t\tthis.items.Clear();\n");
        fprintf(fcs,"\t\tstring basetable = \"%s\";\n",argv[1]);
        fprintf(fcs,"\t\tstring oldvalue = \"\";\n");
        fprintf(fcs,"\t\tstring newvalue = \"\";\n");
        fprintf(fcs,"\t\tAuditLog.ActionType actiontype = AuditLog.ActionType.Update;\n");
        fprintf(fcs,"\t\tif (oNew.%sID > 0){\n",argv[1]);
        count = 0;
        tp = 0;
        for(count==0;count<totRows;count++){
            strcpy(teststring,"DATE");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=4;
            }        
            strcpy(teststring,"DATETIME");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=4;
            }

            if(tp==4){
                fprintf(fcs,"\t\t\tif(oNew.%s != oOld.%s){\n",fields[count],fields[count]);
                fprintf(fcs,"\t\t\t\tnewvalue = (oNew.%s == null) ? \"\": oNew.%s.ToString();\n",fields[count],fields[count]);
                fprintf(fcs,"\t\t\t\toldvalue = (oOld.%s == null) ? \"\" : oOld.%s.ToString();\n",fields[count],fields[count]);
                fprintf(fcs,"\t\t\t\titems.Add(new AuditLog(curUserID, basetable, \"%s\", ObjectID, actiontype, newvalue, oldvalue));\n",fields[count]);
                fprintf(fcs,"\t\t\t}");
            }else{
                fprintf(fcs,"\t\t\tif(oNew.%s != oOld.%s){\n",fields[count],fields[count]);
                fprintf(fcs,"\t\t\t\titems.Add(new AuditLog(curUserID, basetable, \"%s\", ObjectID, actiontype, oNew.%s, oOld.%s));",fields[count],fields[count],fields[count]);
                fprintf(fcs,"\t\t\t}\n");
            }
            
        }
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\tif(items.Count > 0) {\n");
        fprintf(fcs,"\t\t\tforeach(AuditLog item in items){  \n");
        fprintf(fcs,"\t\t\t\t(new AuditLogBL()).save(item);  \n");
        fprintf(fcs,"\t\t\t}  \n");
        fprintf(fcs,"\t\t}  \n");
        fprintf(fcs,"\t}\n");

        
        
        
        //// MS SQL Server Version

        
        fprintf(fcs,"\n\n\n\n /// SQL Server Version of D");
        fprintf(fcs,"public class %sD{\n",argv[1]);
        fprintf(fcs,"\tprotected string _LastErrorD = \"\";\n");
        fprintf(fcs,"\n\tpublic string LastErrorD {get=>_LastErrorD; set=> _LastErrorD=value;}\n\n");
        fprintf(fcs,"\tpublic %s load(int ID, %s o){\n",argv[1],argv[1]);
        fprintf(fcs,"\t\to.%sID = ID;\n",argv[1]);
        fprintf(fcs,"\t\tstring connString = DALGlobal.connectionString; \n");
        fprintf(fcs,"\t\tusing (SqlConnection con = new SqlConnection(connString)){\n");
        fprintf(fcs,"\t\t\tcon.Open();\n");
        fprintf(fcs,"\t\t\to = load(ID, con, o);\n");
        fprintf(fcs,"\t\t\tcon.Close();\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn o;\n");
        fprintf(fcs,"\t}\n");


        fprintf(fcs,"\tpublic %s load(int ID, SqlConnection con, %s o", argv[1], argv[1]);
        if(hasBIN==1){
            fprintf(fcs,",string key");
        }
        fprintf(fcs,"){\n");
        if(hasBIN){
            fprintf(fcs,"\t\tusing(SqlCommand cmdk = new SqlCommand(\"OpenKeys\",con)){\n");
            fprintf(fcs,"\t\t\tcmdk.ExecuteNonQuery();\n");
            fprintf(fcs,"\t\t}\n");
        }
        fprintf(fcs,"\t\t_LastErrorD = \"\";\n");
        fprintf(fcs,"\t\to.%sID = ID;\n",argv[1]);
        fprintf(fcs,"\t\tstring sql = \"SELECT \";\n");
        fprintf(fcs,"\t\tsql += \"%sID \";\n", argv[1]);
        count = 0;
        for(count==0;count<totRows;count++){
            strcpy(teststring,"VARBINARY");
            compresult = strncmp(types[count], teststring,9);
            tp=2;
            if(compresult==0){
                tp=6;
            }
            if(tp==6){
                fprintf(fcs,"\t\tsql += \",dbo.getEData(%s,'\" + key + \"',%sID) as %s \";\n", fields[count],fields[count],argv[1]);
            }else{
                fprintf(fcs,"\t\tsql += \",%s \";\n", fields[count]);
            }
        }
        fprintf(fcs,"\t\tsql += \",RecordDeleted \";\n");
        fprintf(fcs,"\t\tsql += \",RecordLockByUserID \";\n");
        fprintf(fcs,"\t\tsql += \",RecordLockTime \";\n");
        fprintf(fcs,"\t\tsql += \",dateCreated \";\n");
        fprintf(fcs,"\t\tsql += \",dateModified \";\n");
        fprintf(fcs,"\t\tsql += \",ModifiedByUserID \";\n");
        fprintf(fcs,"\t\tsql += \"FROM tbl%s  \";\n", argv[1]);
        fprintf(fcs,"\t\tsql += \"WHERE %sID=\" + o.%sID + \";\";\n", argv[1],argv[1]);
        fprintf(fcs,"\t\ttry{\n");
        fprintf(fcs,"\t\t\tusing(SqlCommand cmd = new SqlCommand(sql,con)){\n");
        fprintf(fcs,"\t\t\t\tSqlDataReader r;\n");
        fprintf(fcs,"\t\t\t\tusing(r = cmd.ExecuteReader()){\n");
        fprintf(fcs,"\t\t\t\t\twhile(r.Read()){\n");
        count = 0;
        compresult = 0;
        tp = 2; // 0 = int, 1 = float, 2 = string, 3=decimal, 4=date
        for(count==0;count<totRows;count++){
            tp = 2; // 0 = int, 1 = float, 2 = string, 3=decimal, 4=date
            strcpy(teststring,"INT");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            }        
            strcpy(teststring,"TINYINT");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            }        
            strcpy(teststring,"int");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            } 

            strcpy(teststring,"DECIMAL(10,2)");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=3;
            }
            strcpy(teststring,"DATE");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=4;
            }        
            strcpy(teststring,"DATETIME");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=4;
            }
            strcpy(teststring,"TIME");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=5;
            }

            if(tp==0){
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? 0 : r.GetInt32(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else if(tp==1){
                // float
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? 0 : r.GetInt32(\"%s\");\n", fields[count], fields[count], fields[count]);
                
            }else if(tp==2){
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? \"\" : r.GetString(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else if(tp==3){
                // double
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? 0 : r.GetDouble(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else if(tp==4){
                // DateTime
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? DateTime.MinValue : r.GetDateTime(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else if(tp==5){
                // Time
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? DateTime.MinValue : r.GetDateTime(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else{
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? \"\" : r.GetString(\"%s\");\n", fields[count], fields[count], fields[count]);
            }
            
        }
        fprintf(fcs,"\t\t\t\t\t\to.RecordDeleted =r.IsDBNull(\"RecordDeleted\") ? false : r.GetInt32(\"RecordDeleted\")==1;\n");
        fprintf(fcs,"\t\t\t\t\t\to.RecordLockByUserID =r.IsDBNull(\"RecordLockByUserID\") ? 0 : r.GetInt32(\"RecordLockByUserID\");\n");
        fprintf(fcs,"\t\t\t\t\t\to.RecordLockTime =r.IsDBNull(\"RecordLockTime\") ? DateTime.MinValue : r.GetDateTime(\"RecordLockTime\");\n");
        fprintf(fcs,"\t\t\t\t\t\to.dateCreated =r.IsDBNull(\"dateCreated\") ? DateTime.MinValue : r.GetDateTime(\"dateCreated\");\n");
        fprintf(fcs,"\t\t\t\t\t\to.dateModified =r.IsDBNull(\"dateModified\") ? DateTime.MinValue : r.GetDateTime(\"dateModified\");\n");
        fprintf(fcs,"\t\t\t\t\t\to.ModifiedByUserID =r.IsDBNull(\"ModifiedByUserID\") ? 0 : r.GetInt32(\"ModifiedByUserID\");\n");


        fprintf(fcs,"\t\t\t\t\t}\n");
        fprintf(fcs,"\t\t\t\t\tr.Close();\n");
        fprintf(fcs,"\t\t\t\t}\n");
        fprintf(fcs,"\t\t\t}\n");
        fprintf(fcs,"\t\t\to.LastError = \"\";\n");
        fprintf(fcs,"\t\t\t_LastErrorD = \"\";\n");
        fprintf(fcs,"\t\t}catch(Exception ex){\n");
        fprintf(fcs,"\t\t\to.LastError = ex.Message;\n");
        fprintf(fcs,"\t\t\t_LastErrorD = ex.Message;\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn o;\n");
        fprintf(fcs,"\t}\n");
        
        fprintf(fcs,"\tpublic %s save(%s o, int curUserID",argv[1],argv[1]);
        if(hasBIN==1){
            fprintf(fcs,", string key");
        }
        fprintf(fcs,"){\n");
        if(hasBIN){
            fprintf(fcs,"\t\tusing(SqlCommand cmdk = new SqlCommand(\"OpenKeys\",con)){\n");
            fprintf(fcs,"\t\t\tcmdk.ExecuteNonQuery();\n");
            fprintf(fcs,"\t\t}\n");
        }
        fprintf(fcs,"\t\t_LastErrorD = \"\";\n");
        fprintf(fcs,"\t\tint ret = 0;\n");
        fprintf(fcs,"\t\tstring connString = DALGlobal.connectionString; \n");
        fprintf(fcs,"\t\tusing (SqlConnection con = new SqlConnection(connString)){\n");
        fprintf(fcs,"\t\t\tcon.Open();\n");
        fprintf(fcs,"\t\t\to = save(con,o,curUserID);\n");
        fprintf(fcs,"\t\t\tcon.Close();\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn o;\n");
        fprintf(fcs,"\t}\n");
        fprintf(fcs,"\n");
        fprintf(fcs,"\tpublic %s save(SqlConnection con, %s o, int curUserID",argv[1],argv[1]);
        if(hasBIN==1){
            fprintf(fcs,",string key");
        }
        fprintf(fcs,"){\n");
        fprintf(fcs,"\t\tint ret = 0;\n");
        if(hasBIN==1){
            fprintf(fcs,"\t\tusing(SqlCommand cmd = new SqlCommand(\"OpenKeys\",con)){\n");
            fprintf(fcs,"\t\t\tcmd.ExecuteNonQuery()\n");
            fprintf(fcs,"\t\t}\n");
        } 
        fprintf(fcs,"\t\tusing(SqlCommand cmd = new SqlCommand(\"sp_Update%s\",con)){\n", argv[1]);
        fprintf(fcs,"\t\t\tcmd.CommandType = CommandType.StoredProcedure;\n");
        fprintf(fcs,"\t\t\tcmd.Parameters.AddWithValue(\"@%sID\",o.%sID);\n",argv[1],argv[1]);
        count = 0;
        for(count==0;count<totRows;count++){
            tp = 2; // 0 = int, 1 = float, 2 = string, 3=decimal, 4=date
            strcpy(teststring,"INT");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            }        
            strcpy(teststring,"TINYINT");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            }        
            strcpy(teststring,"int");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            } 

            strcpy(teststring,"DECIMAL(10,2)");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=3;
            }
            strcpy(teststring,"DATE");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=4;
            }        
            strcpy(teststring,"DATETIME");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=4;
            }
            strcpy(teststring,"TIME");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=5;
            }

            if(tp==4){

                fprintf(fcs,"\t\t\tif(o.%s<new DateTime(1753,1,1)){\n",fields[count]);
                fprintf(fcs,"\t\t\t\tcmd.Parameters.AddWithValue(\"@%s\",DBNull.Value);\n",fields[count]);
                fprintf(fcs,"\t\t\t}else{\n");
                fprintf(fcs,"\t\t\t\tcmd.Parameters.AddWithValue(\"@%s\",o.%s);\n",fields[count],fields[count]);
                fprintf(fcs,"\t\t}else{\n");

            }else{
                fprintf(fcs,"\t\t\tcmd.Parameters.AddWithValue(\"@%s\",o.%s);\n",fields[count],fields[count]);

            }
        }
        fprintf(fcs,"\t\t\tcmd.Parameters.AddWithValue(\"@CurrentUserID\",curUserID);\n");
        fprintf(fcs,"\t\t\ttry{\n");
        fprintf(fcs,"\t\t\t\tvar recid = cmd.ExecuteScalar();\n");
        fprintf(fcs,"\t\t\t\tint.TryParse(recid.ToString(), out ret);\n");
        fprintf(fcs,"\t\t\t}catch (Exception ex){\n");
        fprintf(fcs,"\t\t\t\to.LastError = ex.Message;\n");
        fprintf(fcs,"\t\t\t\t_LastErrorD = ex.Message;\n");
        fprintf(fcs,"\t\t\t\tret = -1;\n");
        fprintf(fcs,"\t\t\t}\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\n");

        fprintf(fcs,"\t\to.%sID=ret;;\n",argv[1]);
        fprintf(fcs,"\t\treturn o;\n");
        fprintf(fcs,"\t}\n");

        fprintf(fcs,"\tpublic int delete(%s o, int curUserID){\n",argv[1]);
        fprintf(fcs,"\t\tint ret = 0;\n");
        fprintf(fcs,"\t\tstring connString = DALGlobal.connectionString; \n");
        fprintf(fcs,"\t\tusing (SqlConnection con = new SqlConnection(connString)){\n");
        fprintf(fcs,"\t\t\tcon.Open();\n");
        fprintf(fcs,"\t\t\tret = delete(con,o, curUserID);\n");
        fprintf(fcs,"\t\t\tcon.Close();\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn ret;\n");
        fprintf(fcs,"\t}\n");

        fprintf(fcs,"\tpublic int delete(SqlConnection con, %s o, int curUserID){\n",argv[1]);
        fprintf(fcs,"\t\tAuditLog a = new AuditLog(curUserID, \"%s\", \"%sID\",o.%sID, AuditLog.ActionType.Delete, o.%sID, 0);\n",argv[1],argv[1],argv[1],argv[1]);
        fprintf(fcs,"\t\t(new AuditLogD()).save(a);\n");
        fprintf(fcs,"\t\tint ret = 0;\n");
        fprintf(fcs,"\t\tusing (SqlCommand cmd = new SqlCommand(\"sp_Delete%s\", con)){\n", argv[1]);
        fprintf(fcs,"\t\t\tcmd.CommandType = CommandType.StoredProcedure;\n");
        fprintf(fcs,"\t\t\tcmd.Parameters.AddWithValue(\"@%sID\",o.%sID);\n",argv[1],argv[1]);
        fprintf(fcs,"\t\t\tcmd.Parameters.AddWithValue(\"@CurrentUserID\",curUserID);\n");
        fprintf(fcs,"\t\t\ttry{\n");
        fprintf(fcs,"\t\t\t\tvar recid = cmd.ExecuteNonQuery();\n");
        fprintf(fcs,"\t\t\t\tint.TryParse(recid.ToString(), out ret);\n");
        fprintf(fcs,"\t\t\t}catch (Exception ex){\n");
        fprintf(fcs,"\t\t\t\t_LastErrorD = ex.Message;\n");
        fprintf(fcs,"\t\t\t\tret = -1;\n");
        fprintf(fcs,"\t\t\t}\n");
        
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn ret;\n");
        fprintf(fcs,"\t}\n\n\n");
        
        fprintf(fcs,"\tpublic List<%s> get%ssL(){\n",argv[1],argv[1]);
        fprintf(fcs,"\t\tList<%s> l = new List<%s>();\n",argv[1],argv[1]);
        fprintf(fcs,"\t\tstring connString = DALGlobal.connectionString; \n");
        fprintf(fcs,"\t\tusing (SqlConnection con = new SqlConnection(connString)){\n");
        fprintf(fcs,"\t\t\tcon.Open();\n");
        fprintf(fcs,"\t\t\tl = get%ssL(con);\n",argv[1]);
        fprintf(fcs,"\t\t\tcon.Close();\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn l;\n");
        fprintf(fcs,"\t}\n");
                    
        fprintf(fcs,"\tpublic List<%s> get%ssL(SqlConnection con){\n", argv[1], argv[1]);
        if(hasBIN){
            fprintf(fcs,"\t\tusing(SqlCommand cmdk = new SqlCommand(\"OpenKeys\",con)){\n");
            fprintf(fcs,"\t\t\tcmdk.ExecuteNonQuery();\n");
            fprintf(fcs,"\t\t}\n");
        }

        fprintf(fcs,"\t\tList<%s> l = new List<%s>();\n",argv[1],argv[1]);
        fprintf(fcs,"\t\t_LastErrorD = \"\";\n");
        fprintf(fcs,"\t\tstring sql = \"SELECT \";\n");
        fprintf(fcs,"\t\tsql += \"%sID \";\n", argv[1]);
        count = 0;
        for(count==0;count<totRows;count++){
            strcpy(teststring,"VARBINARY");
            compresult = strncmp(types[count], teststring,9);
            tp=2;
            if(compresult==0){
                tp=6;
            }
            if(tp==6){
                fprintf(fcs,"\t\tsql += \",dbp.getEData(%s,ES) as %s \";\n", fields[count],fields[count]);
            }else{
                fprintf(fcs,"\t\tsql += \",%s \";\n", fields[count]);
            }
        }
        fprintf(fcs,"\t\tsql += \",RecordDeleted \";\n");
        fprintf(fcs,"\t\tsql += \",RecordLockByUserID \";\n");
        fprintf(fcs,"\t\tsql += \",RecordLockTime \";\n");
        fprintf(fcs,"\t\tsql += \",dateCreated \";\n");
        fprintf(fcs,"\t\tsql += \",dateModified \";\n");
        fprintf(fcs,"\t\tsql += \",ModifiedByUserID \";\n");
        fprintf(fcs,"\t\tsql += \" FROM tbl%s  \";\n", argv[1]);
        fprintf(fcs,"\t\tsql += \" WHERE RecordDeleted=0  \";\n");
        fprintf(fcs,"\t\t//sql += \"AND %sID=\" + o.%sID + \";\";\n", argv[1],argv[1]);
        fprintf(fcs,"\t\ttry{\n");
        fprintf(fcs,"\t\t\tusing(SqlCommand cmd = new SqlCommand(sql,con)){\n");
        fprintf(fcs,"\t\t\t\tSqlDataReader r;\n");
        fprintf(fcs,"\t\t\t\tusing(r = cmd.ExecuteReader()){\n");
        fprintf(fcs,"\t\t\t\t\twhile(r.Read()){\n");
        fprintf(fcs,"\t\t\t\t\t\t%s o = new %s();\n",argv[1],argv[1]);
        fprintf(fcs,"\t\t\t\t\t\to.%sID =r.IsDBNull(\"%sID\") ? 0 : r.GetInt32(\"%sID\");\n",argv[1],argv[1],argv[1]);
        count = 0;
        compresult = 0;
        tp = 2; // 0 = int, 1 = float, 2 = string, 3=decimal, 4=date
        for(count==0;count<totRows;count++){
            tp = 2; // 0 = int, 1 = float, 2 = string, 3=decimal, 4=date
            strcpy(teststring,"INT");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            }        
            strcpy(teststring,"TINYINT");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            }        
            strcpy(teststring,"int");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            } 

            strcpy(teststring,"DECIMAL(10,2)");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=3;
            }
            strcpy(teststring,"DATE");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=4;
            }        
            strcpy(teststring,"DATETIME");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=4;
            }
            strcpy(teststring,"TIME");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=5;
            }

            if(tp==0){
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? 0 : r.GetInt32(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else if(tp==1){
                // float
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? 0 : r.GetInt32(\"%s\");\n", fields[count], fields[count], fields[count]);
                
            }else if(tp==2){
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? \"\" : r.GetString(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else if(tp==3){
                // double
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? 0 : r.GetInt32(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else if(tp==4){
                // DateTime
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? DateTime.MinValue : r.GetDateTime(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else if(tp==5){
                // Time
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? DateTime.MinValue : r.GetDateTime(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else{
                fprintf(fcs,"\t\t\t\t\t\to.%s =r.IsDBNull(\"%s\") ? \"\" : r.GetString(\"%s\");\n", fields[count], fields[count], fields[count]);
            }
            
        }
        fprintf(fcs,"\t\t\t\t\t\to.RecordDeleted =r.IsDBNull(\"RecordDeleted\") ? false : r.GetInt32(\"RecordDeleted\")==1;\n");
        fprintf(fcs,"\t\t\t\t\t\to.RecordLockByUserID =r.IsDBNull(\"RecordLockByUserID\") ? 0 : r.GetInt32(\"RecordLockByUserID\");\n");
        fprintf(fcs,"\t\t\t\t\t\to.RecordLockTime =r.IsDBNull(\"RecordLockTime\") ? DateTime.MinValue : r.GetDateTime(\"RecordLockTime\");\n");
        fprintf(fcs,"\t\t\t\t\t\to.dateCreated =r.IsDBNull(\"dateCreated\") ? DateTime.MinValue : r.GetDateTime(\"dateCreated\");\n");
        fprintf(fcs,"\t\t\t\t\t\to.dateModified =r.IsDBNull(\"dateModified\") ? DateTime.MinValue : r.GetDateTime(\"dateModified\");\n");
        fprintf(fcs,"\t\t\t\t\t\to.ModifiedByUserID =r.IsDBNull(\"ModifiedByUserID\") ? 0 : r.GetInt32(\"ModifiedByUserID\");\n");


        fprintf(fcs,"\t\t\t\t\t\tl.Add(o);\n");
        fprintf(fcs,"\t\t\t\t\t}\n");
        fprintf(fcs,"\t\t\t\t\tr.Close();\n");
        fprintf(fcs,"\t\t\t\t}\n");
        fprintf(fcs,"\t\t\t}\n");
        fprintf(fcs,"\t\t\t_LastErrorD = \"\";\n");
        fprintf(fcs,"\t\t}catch(Exception ex){\n");
        fprintf(fcs,"\t\t\t_LastErrorD = ex.Message;\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn l;\n");
        fprintf(fcs,"\t}\n");
       



        fprintf(fcs,"\tpublic DataTable get%ssDT(string srchString, List<UserViewField> fields,string SortBy, string SortDirection",argv[1]);
        if(hasBIN==1){
            fprintf(fcs,", string key");
        }
        fprintf(fcs,"){\n");
        fprintf(fcs,"\t\tDataTable dt = new DataTable();\n");
        fprintf(fcs,"\t\tstring connString = DALGlobal.connectionString; \n");
        fprintf(fcs,"\t\tusing (SqlConnection con = new SqlConnection(connString)){\n");
        fprintf(fcs,"\t\t\tcon.Open();\n");
        fprintf(fcs,"\t\t\tdt = get%ssDT(con,srchString, fields, SortBy, SortDirection",argv[1]);
        if(hasBIN==1){
            fprintf(fcs,",key = \"\"");
        }
        fprintf(fcs,");\n");
        fprintf(fcs,"\t\t\tcon.Close();\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn dt;\n");
        fprintf(fcs,"\t}\n");
                    
        fprintf(fcs,"\tprivate DataTable get%ssDT(SqlConnection con,string srchString, List<UserViewField> fields, string SortBy, string SortDirection", argv[1]);
        if(hasBIN==1){
            fprintf(fcs,", string key");
        }
        fprintf(fcs,"){\n");
        if(hasBIN){
            fprintf(fcs,"\t\tusing(SqlCommand cmdk = new SqlCommand(\"OpenKeys\",con)){\n");
            fprintf(fcs,"\t\t\tcmdk.ExecuteNonQuery();\n");
            fprintf(fcs,"\t\t}\n");
        }
        
        fprintf(fcs,"\t\tDataTable dt = new DataTable();\n");
        fprintf(fcs,"\t\t_LastErrorD = \"\";\n");
        fprintf(fcs,"\t\tstring sql = \"SELECT \";\n");
        fprintf(fcs,"\t\tstring strFields = \"\";\n");
        fprintf(fcs,"\t\tstring strIncKey = \"\";\n");
        fprintf(fcs,"\t\tstring strExtraJoins = \"\";\n");

        fprintf(fcs,"\t\tif(fields.Count>0){\n");
        fprintf(fcs,"\t\t\tforeach(UserViewField f in fields){\n");
            fprintf(fcs,"\t\t\t\tif(f.FieldName==\"tmpGUID\"){\n");
            fprintf(fcs,"\t\t\t\t\tstrFields += \",CAST(SELECT NEWID() AS VARCHAR(100)) as tmpGUID \";\n");
            fprintf(fcs,"\t\t\t\t}\n");
        count = 0;
        posss = 0;
        for(count==0;count<totRows;count++){
            strcpy(teststring,"VARBINARY");
            compresult = strncmp(types[count], teststring,9);
            tp=2;
            if(compresult==0){
                tp=6;
                fprintf(fcs,"\t\t\t\telse if(f.FieldName==\"%s\"){\n",fields[count]);
                fprintf(fcs,"\t\t\t\t\tstrFields += \", dbo.getEData(%s,ES) as %s\";\n",fields[count],fields[count]);
                fprintf(fcs,"\t\t\t\t}\n");
                posss ++;
            }
        }
        fprintf(fcs,"\t\t\t\telse{\n");
        fprintf(fcs,"\t\t\t\t\tstrFields += \", \" + f.FieldName + \" \";\n");
        fprintf(fcs,"\t\t\t\t\t//extraJoins += \" LEFT JOIN tblOtherTable as t on t.XID=x.XID \"\n");
        fprintf(fcs,"\t\t\t\t}\n");
        fprintf(fcs,"\t\t\t}\n");
        fprintf(fcs,"\t\t}else{\n");
        count = 0;
        for(count==0;count<totRows;count++){
            strcpy(teststring,"VARBINARY");
            compresult = strncmp(types[count], teststring,9);
            tp=2;
            if(compresult==0){
                tp=6;
            }
            if(count==0){
                /// leave off the comma
                if(tp==6){
                    fprintf(fcs,"\t\t\tstrFields += \"setEData(%s,ES) as %s \";\n", fields[count],fields[count]);
                }else{
                    fprintf(fcs,"\t\t\tsql += \"%s \";\n", fields[count]);
                }
            }else{
                if(tp==6){
                    fprintf(fcs,"\t\t\tsql += \",setEData(%s,ES) as %s \";\n", fields[count],fields[count]);
                }else{
                    fprintf(fcs,"\t\t\tsql += \",%s \";\n", fields[count]);
                }
            }
        }
        fprintf(fcs,"\t\t\tstrFields += \",RecordDeleted \";\n");
        fprintf(fcs,"\t\t\tstrFields += \",RecordLockByUserID \";\n");
        fprintf(fcs,"\t\t\tstrFields += \",RecordLockTime \";\n");
        fprintf(fcs,"\t\t\tstrFields += \",dateCreated \";\n");
        fprintf(fcs,"\t\t\tstrFields += \",dateModified \";\n");
        fprintf(fcs,"\t\t\tstrFields += \",ModifiedByUserID \";\n");
        fprintf(fcs,"\t\t}\n");

        fprintf(fcs,"\t\tstring sortString = \"\";\n");
        fprintf(fcs,"\t\tif(SortBy.Length>0){\n");
        count = 0;
        posss = 0;
        for(count==0;count<totRows;count++){
            strcpy(teststring,"VARBINARY");
            compresult = strncmp(types[count], teststring,9);
            tp=2;
            if(compresult==0){
                tp=6;
                if(posss==0){
                    fprintf(fcs,"\t\t\t\tif(SortBy==\"%s\"){\n",fields[count]);
                }else{
                    fprintf(fcs,"\t\t\t\telse if(SortBy==\"%s\"){\n",fields[count]);
                }
                fprintf(fcs,"\t\t\t\t\tSortBy = \"dbo.getEData(%s,ES) \";\n",fields[count]);
                fprintf(fcs,"\t\t\t\t}\n");
                posss ++;
            }
        }
        fprintf(fcs,"\t\t\tsql += \" ORDER BY \" + SortBy + \" \" + SortDirection + \" \";\n");
        fprintf(fcs,"\t\t}\n");

        fprintf(fcs,"\t\tif(SortBy.Length>0){\n");
        fprintf(fcs,"\t\t\tsortString += \" ORDER BY \" + SortBy + \" \" + SortDirection + \" \";\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\tsql += strIncKey + \"%sID \" + strFields + \" \";\n", argv[1]);
        fprintf(fcs,"\t\tsql += \" FROM tbl%s  \";\n", argv[1]);
        fprintf(fcs,"\t\tsql += \" \" + strExtraJoins + \" \";\n");
        fprintf(fcs,"\t\tsql += \" WHERE tbl%s.RecordDeleted=0  \";\n",argv[1]);
        fprintf(fcs,"\t\tif(srchString.Length>0){\n");
        fprintf(fcs,"\t\t\tsql += \" AND Description LIKE '%%\" + srchString + \"%%'\";\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\tsql += \" \" + sortString +  \" \";\n");

        fprintf(fcs,"\t\ttry{\n");
        if(hasBIN==1){
            fprintf(fcs,"\t\tusing(SqlCommand cmd = new SqlCommand(\"OpenKeys\",con)){\n");
            fprintf(fcs,"\t\t\tcmd.ExecuteNonQuery();\n");
            fprintf(fcs,"\t\t}\n");
        } 
        fprintf(fcs,"\t\t\tusing(SqlDataAdapter da = new SqlDataAdapter(sql,con)){\n");
        fprintf(fcs,"\t\t\t\tdt = new DataTable();\n");
        fprintf(fcs,"\t\t\t\tda.Fill(dt);\n");
        fprintf(fcs,"\t\t\t\t_LastErrorD = \"\";\n");
        fprintf(fcs,"\t\t\t}\n");
        fprintf(fcs,"\t\t}catch(Exception ex){\n");
        fprintf(fcs,"\t\t\t_LastErrorD = ex.Message;\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn dt;\n");
        fprintf(fcs,"\t}\n");


        fprintf(fcs,"}\n\n\n");









        fclose(fcs);
    }













    //// create the helper cs form procedures

    char filenamecsx[128];
    snprintf(filenamecsx, sizeof filenamecsx, "%s.frm.cs",argv[1]);


    FILE *fcsx;
    fcsx = fopen(filenamecsx,"w");

    count = 0;

    if(fcsx!=NULL){
        
        fprintf(fcsx,"\n\n");
        fprintf(fcsx,"\tint %sID;\n",argv[1]);
        fprintf(fcsx,"\t%s Obj%s;\n\n\n",argv[1],argv[1]);
        fprintf(fcsx,"\tprivate List<UserView> views = new List<UserView>();\n");
        fprintf(fcsx,"\tUserView selectedView = null;\n");
        fprintf(fcsx,"\tpublic frm%s(){\n",argv[1]);
        fprintf(fcsx,"\t\tInitializeComponent();\n");
        fprintf(fcsx,"\t\tappglobal.applyTheme(this);\n");
        fprintf(fcsx,"\t\tloadViewsMenu();\n");
        fprintf(fcsx,"\t}\n");
        fprintf(fcsx,"\tpublic event EventHandler<CloseFormEventArgs> SaveComplete;\n\n");
        fprintf(fcsx,"\tpublic void load%s(int ID){\n",argv[1]);
        fprintf(fcsx,"\t\tthis.%sID=ID;\n",argv[1]);
        fprintf(fcsx,"\t\tthis.Obj%s = new %s();\n",argv[1],argv[1]);
        fprintf(fcsx,"\t\t%sbl = new %sBL();\n",argv[1],argv[1]);

        fprintf(fcsx,"\t\tObj%s = %sbl.load%s(ID,Obj%s",argv[1],argv[1],argv[1],argv[1]);
        if(hasBIN==1){
            fprintf(fcs,",appglobal.Key");
        }
        fprintf(fcs,");\n");
        fprintf(fcsx,"\t\tif(%sbl.LastErrorB.Length > 1){\n",argv[1]);
        fprintf(fcsx,"\t\t\tErrorLogBL.addLog(appglobal.curUserID,\"ERROR F%s-001 Unable to load %s: \" + %sbl.LastErrorB);\n",argv[1],argv[1],argv[1]);
        fprintf(fcsx,"\t\t\tMessageBox.Show(\"ERROR F%s-001 Unable to load %s: \" + %sbl.LastErrorB);\n",argv[1],argv[1],argv[1]);
        fprintf(fcsx,"\t\t}else{\n");


        count = 0;
        for(count==0;count<totRows;count++){
            tp = 2; // 0 = int, 1 = float, 2 = string, 3=decimal, 4=date
            strcpy(teststring,"DATE");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=4;
            }        
            strcpy(teststring,"DATETIME");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=4;
            }
            strcpy(teststring,"TIME");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=4;
            }

            if(tp==4){
                fprintf(fcsx,"\t\t\tif(%s.%s > DateTime.MinValue){\n",argv[1],fields[count]);
                fprintf(fcsx,"\t\t\t\tthis.txt%s.Text = Obj%s.%s.ToString(\"MM/dd/yyyy\");\n", fields[count],argv[1],fields[count]);
                fprintf(fcsx,"\t\t\t}\n");
            }else{
                fprintf(fcsx,"\t\t\tthis.txt%s.Text = Obj%s.%s;\n", fields[count],argv[1],fields[count]);
            }
                
        }
        
        fprintf(fcsx,"\t\t}\n");
        fprintf(fcsx,"\t}\n");
        fprintf(fcsx,"\tpublic void save%s(bool closeOnComplete=true){\n",argv[1]);
        count = 0;
        compresult = 0;
        tp = 2; // 0 = int, 1 = float, 2 = string, 3=decimal, 4=date
        for(count==0;count<totRows;count++){
            tp = 2; // 0 = int, 1 = float, 2 = string, 3=decimal, 4=date
            strcpy(teststring,"DATE");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=4;
            }        
            strcpy(teststring,"DATETIME");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=4;
            }
            strcpy(teststring,"TIME");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=4;
            }

            if(tp==4){
                fprintf(fcsx,"\t\tDateTime.TryParse(this.txt%s.Text + "", out Obj%s.%s);\n",fields[count],argv[1],fields[count]);
            }else{
                fprintf(fcsx,"\t\tObj%s.%s = this.txt%s.Text;\n", argv[1],fields[count],fields[count]);
            }
                
        }
        fprintf(fcsx,"\t\tbool wasinsert = true;\n");
        fprintf(fcsx,"\t\t%sBL %sbl = new %sBL();\n",argv[1],argv[1],argv[1]);
        fprintf(fcsx,"\t\tif(Obj%s.%sID>0){\n",argv[1],argv[1]);
        fprintf(fcsx,"\t\t\twasinsert = false;\n");
        fprintf(fcsx,"\t\t\t%s oOld = %sbl.load%s(Obj%s.%sID,oOld",argv[1],argv[1],argv[1],argv[1],argv[1]);
        if(hasBIN==1){
            fprintf(fcs,",appglobal.Key");
        }
        fprintf(fcs,");\n");
        fprintf(fcsx,"\t\t\tif(oOld.%sID>0){\n",argv[1]);
        fprintf(fcsx,"\t\t\t%sbl.setHistory(cOld,%s,appglobal.curUserID);\n",argv[1],argv[1]);
        fprintf(fcsx,"\t\t\t}\n");
        fprintf(fcsx,"\t\t}\n");

        fprintf(fcsx,"\t\t%sbl.LastErrorB = \"\";\n",argv[1]);
        fprintf(fcsx,"\t\tObj%s = %sbl.save(Obj%s,appglobal.curUserID",argv[1],argv[1],argv[1]);
        if(hasBIN==1){
            fprintf(fcs,",appglobal.Key");
        }
        fprintf(fcs,");\n");
        fprintf(fcsx,"\t\tif(%sbl.LastErrorB.Length>0){\n",argv[1]);
        fprintf(fcsx,"\t\t\tErrorLogBL.addLog(appglobal.curUserID,\"ERROR F%s-002 Unable to save %s: \" + %sbl.LastErrorB);\n",argv[1],argv[1],argv[1]);
        fprintf(fcsx,"\t\t\tMessageBox.Show(\"ERROR F%s-002 Unable to save %s: \" + %sbl.LastErrorB);\n",argv[1],argv[1],argv[1]);

        fprintf(fcsx,"\t\t}else{\n");

        fprintf(fcsx,"\t\t\tif(wasinsert){\n");
        fprintf(fcsx,"\t\t\t\t%sbl.LastErrorB = \"\";\n",argv[1]);
        fprintf(fcsx,"\t\t\t\t%sbl.setHistory(Obj%s,appglobal.curUserID);\n",argv[1],argv[1]);
        fprintf(fcsx,"\t\t\t\tif(%sbl.LastErrorB.Length>0){\n",argv[1]);
        fprintf(fcsx,"\t\t\t\t\tErrorLogBL.addLog(appglobal.curUserID,\"ERROR F%s-003 Unable to set history %s: \" + %sbl.LastErrorB);\n",argv[1],argv[1],argv[1]);
        fprintf(fcsx,"\t\t\t\t\tMessageBox.Show(\"ERROR F%s-002 Unable to set history %s: \" + %sbl.LastErrorB);\n",argv[1],argv[1],argv[1]);
        fprintf(fcsx,"\t\t\t\t}\n");
        fprintf(fcsx,"\t\t\t}\n");

        fprintf(fcsx,"\t\t\tEventHandler<CloseFormEventArgs> handler = this.SaveComplete;\n");
        fprintf(fcsx,"\t\t\tif(handler != null){\n");
        fprintf(fcsx,"\t\t\t\tCloseFormEventArgs e = new CloseFormEventArgs();\n");
        fprintf(fcsx,"\t\t\t\te.CompletedAction = (wasinsert?CloseFormEventArgs.FormAction.RecordSavedInsert:CloseFormEventArgs.FormAction.RecordSavedUpdate);\n");
        fprintf(fcsx,"\t\t\t\thandler(this,e);\n");
        fprintf(fcsx,"\t\t\t\tif(closeOnComplete){\n");
        fprintf(fcsx,"\t\t\t\t\tthis.Close();\n");
        fprintf(fcsx,"\t\t\t\t}\n");

        fprintf(fcsx,"\t\t\t}\n");
        fprintf(fcsx,"\t\t}\n");
        fprintf(fcsx,"\t}\n");


        fprintf(fcsx,"///put into register version ///\n");
        fprintf(fcsx,"\tint FormTypeID = (int)Globals.FormViewTypes.XXX;\n");
        fprintf(fcsx,"\tstring curSortBy = \"Description\";\n");
        fprintf(fcsx,"\tstring curSortDirection = \"ASC\";\n");
        fprintf(fcsx,"\tpublic frm%s(){\n",argv[1]);
        fprintf(fcsx,"\t\tInitializeComponent();\n");
        fprintf(fcsx,"\t\tappglobal.applyTheme(this);\n");
        fprintf(fcsx,"\t\tloadViewsMenu();\n");
        fprintf(fcsx,"\t\tif (appglobal.menuSecurity.M%s_RW == false)\n",argv[1]);
        fprintf(fcsx,"\t\t{\n");
        fprintf(fcsx,"\t\taddNewToolStripMenuItem.Visible = false;\n");
        fprintf(fcsx,"\t\t}\n");
        fprintf(fcsx,"\t\telse\n");
        fprintf(fcsx,"\t\t{\n");
        fprintf(fcsx,"\t\taddNewToolStripMenuItem.Visible = true;\n");
        fprintf(fcsx,"\t\t}\n");

        fprintf(fcsx,"\t}\n");

        fprintf(fcsx,"\tprivate void loadViewsMenu(){\n");
        fprintf(fcsx,"\t\tviewToolStripMenuItem.DropDownItems.Clear();\n");
        fprintf(fcsx,"\t\tviewToolStripMenuItem.DropDownItems.Add(new ToolStripMenuItem(\"Standard\", null, ApplyView_Click,\"mnuVStandard\"));\n");
        fprintf(fcsx,"\t\tUserViewBL b = new UserViewBL();\n");
        fprintf(fcsx,"\t\tviews = b.getUserViewsL((int)Globals.FormViewTypes.Customer, appglobal.curUserID);\n");
        fprintf(fcsx,"\t\tforeach(UserView view in views){\n");
        fprintf(fcsx,"\t\t\tviewToolStripMenuItem.DropDownItems.Add(new ToolStripMenuItem(view.Description, null, ApplyView_Click, \"mnuV\" + view.UserViewID));\n");
        fprintf(fcsx,"\t\t}\n");
        fprintf(fcsx,"\t}\n");
        fprintf(fcsx,"\tprivate void ApplyView_Click(object? sender, EventArgs e){\n");    
        fprintf(fcsx,"\t\tif (sender != null){\n");
        fprintf(fcsx,"\t\t\tToolStripItem i = (ToolStripItem)sender;\n");
        fprintf(fcsx,"\t\t\tif (i.Text == \"Standard\")\n");
        fprintf(fcsx,"\t\t\t\t{\n");
        fprintf(fcsx,"\t\t\t\t\tselectedView = null;\n");
        fprintf(fcsx,"\t\t\t\t\tsetViewToStandard();\n");
        fprintf(fcsx,"\t\t\t\t\tdoSearch(txtSearch.Text.Trim());\n");
        fprintf(fcsx,"\t\t\t\t}\n");
        fprintf(fcsx,"\t\t\t\telse\n");
        fprintf(fcsx,"\t\t\t\t{\n");
        fprintf(fcsx,"\t\t\t\t\tint viewid = 0;\n");
        fprintf(fcsx,"\t\t\t\t\tif (int.TryParse(i.Name.Replace(\"mnuV\",\"\"), out viewid))\n");
        fprintf(fcsx,"\t\t\t\t\t{\n");
        fprintf(fcsx,"\t\t\t\t\t\tif (viewid > 0)\n");
        fprintf(fcsx,"\t\t\t\t\t\t{\n");
        fprintf(fcsx,"\t\t\t\t\t\t\tUserView? v = views.FindLast(c => c.UserViewID == viewid);\n");
        fprintf(fcsx,"\t\t\t\t\t\t\tif(v != null)\n");
        fprintf(fcsx,"\t\t\t\t\t\t\t{\n");
        fprintf(fcsx,"\t\t\t\t\t\t\t\tsetView(v);\n");
        fprintf(fcsx,"\t\t\t\t\t\t\t\tdoSearch(txtSearch.Text.Trim());\n");
        fprintf(fcsx,"\t\t\t\t\t\t\t}\n");
        fprintf(fcsx,"\t\t\t\t\t\t}\n");
        fprintf(fcsx,"\t\t\t\t\t}\n");
        fprintf(fcsx,"\t\t\t\t\tif (int.TryParse(i.Name.Replace(\"mnuVTop\",\"\"), out viewid))\n");
        fprintf(fcsx,"\t\t\t\t\t{\n");
        fprintf(fcsx,"\t\t\t\t\t\tif (viewid > 0)\n");
        fprintf(fcsx,"\t\t\t\t\t\t{\n");
        fprintf(fcsx,"\t\t\t\t\t\t\tUserView? v = views.FindLast(c => c.UserViewID == viewid);\n");
        fprintf(fcsx,"\t\t\t\t\t\t\tif(v != null)\n");
        fprintf(fcsx,"\t\t\t\t\t\t\t{\n");
        fprintf(fcsx,"\t\t\t\t\t\t\t\tsetView(v);\n");
        fprintf(fcsx,"\t\t\t\t\t\t\t\tdoSearch(txtSearch.Text.Trim());\n");
        fprintf(fcsx,"\t\t\t\t\t\t\t}\n");
        fprintf(fcsx,"\t\t\t\t\t\t}\n");
        fprintf(fcsx,"\t\t\t\t\t}\n");

        fprintf(fcsx,"\t\t\t\t}\n");
        fprintf(fcsx,"\t\t}\n");
        fprintf(fcsx,"\t}\n");

        

        fprintf(fcsx,"\tprivate void setView(UserView v)\n");
        fprintf(fcsx,"\t{\n");
        fprintf(fcsx,"\t\tselectedView = v;\n");
        fprintf(fcsx,"\t\tUserViewFieldBL uvbl = new UserViewFieldBL();\n");
        fprintf(fcsx,"\t\tselectedView.Fields = uvbl.getUserViewFields(selectedView.UserViewID);\n");
        fprintf(fcsx,"\t\tselectedView.Initialised = false;\n");
        fprintf(fcsx,"\t}\n");
        fprintf(fcsx,"\tprivate void formatViewTable()\n");
        fprintf(fcsx,"\t{\n");
        fprintf(fcsx,"\t\tforeach(UserViewField field in selectedView.Fields)\n");
        fprintf(fcsx,"\t\t{\n");
        fprintf(fcsx,"\t\t\ttry\n");
        fprintf(fcsx,"\t\t\t{\n");
        fprintf(fcsx,"\t\t\t\tdataGridView1.Columns[field.FieldName].HeaderText = field.DisplayName;\n");
        fprintf(fcsx,"\t\t\t}\n");
        fprintf(fcsx,"\t\t\t\tcatch(Exception ex)\n");
        fprintf(fcsx,"\t\t\t{\n");
        fprintf(fcsx,"\t\t\t}\n");
        fprintf(fcsx,"\t\t\ttry\n");
        fprintf(fcsx,"\t\t\t{\n");
        fprintf(fcsx,"\t\t\t\tif(field.DisplayAlign==1){\n");
        fprintf(fcsx,"\t\t\t\t\tdataGridView1.Columns[field.FieldName].DefaultCellStyle.Alignment = DataGridViewContentAlignment.TopCenter;\n");
        fprintf(fcsx,"\t\t\t\t}\n");
        fprintf(fcsx,"\t\t\t\tif(field.DisplayFormat.Length > 0){\n");
        fprintf(fcsx,"\t\t\t\t\tdataGridView1.Columns[field.FieldName].DefaultCellStyle.Format = field.DisplayFormat;\n");
        fprintf(fcsx,"\t\t\t\t}\n");
        fprintf(fcsx,"\t\t\t}\n");
        fprintf(fcsx,"\t\t\tcatch(Exception ex)\n");
        fprintf(fcsx,"\t\t\t{\n");
        fprintf(fcsx,"\t\t\t}\n");

        fprintf(fcsx,"\t\t\ttry\n");
        fprintf(fcsx,"\t\t\t{\n");
        fprintf(fcsx,"\t\t\t\tif (field.DisplayColour > 0 && field.DisplayColour < 10)\n");
        fprintf(fcsx,"\t\t\t\t{\n");
        fprintf(fcsx,"\t\t\t\t\tif(appglobal.themeViewColumnBackground[field.DisplayColour] != null)\n");
        fprintf(fcsx,"\t\t\t\t\t{\n");
        fprintf(fcsx,"\t\t\t\t\t\tdataGridView1.Columns[field.FieldName].DefaultCellStyle.BackColor = (Color)appglobal.themeViewColumnBackground[field.DisplayColour];\n");
        fprintf(fcsx,"\t\t\t\t\t}\n");
        fprintf(fcsx,"\t\t\t\t}\n");
        fprintf(fcsx,"\t\t\t}\n");
        fprintf(fcsx,"\t\t\tcatch(Exception ex)\n");
        fprintf(fcsx,"\t\t\t{\n");
        fprintf(fcsx,"\t\t\t}\n");




        fprintf(fcsx,"\t\t\ttry\n");
        fprintf(fcsx,"\t\t\t{\n");
        fprintf(fcsx,"\t\t\t\tif (field.Visible == 0)\n");
        fprintf(fcsx,"\t\t\t\t{\n");
        fprintf(fcsx,"\t\t\t\t\tdataGridView1.Columns[field.FieldName].Visible = false;\n");
        fprintf(fcsx,"\t\t\t\t}\n");
        fprintf(fcsx,"\t\t\t\telse if (field.Visible == 1)\n");
        fprintf(fcsx,"\t\t\t\t{\n");
        fprintf(fcsx,"\t\t\t\t\tdataGridView1.Columns[field.FieldName].Visible = true;\n");
        fprintf(fcsx,"\t\t\t\t}\n");
        fprintf(fcsx,"\t\t\t\telse if (field.Width > 0)\n");
        fprintf(fcsx,"\t\t\t\t{\n");
        fprintf(fcsx,"\t\t\t\t\tdataGridView1.Columns[field.FieldName].Width = (int)field.Width;\n");
        fprintf(fcsx,"\t\t\t\t}\n");
        
        fprintf(fcsx,"\t\t\t}\n");
        fprintf(fcsx,"\t\t\tcatch(Exception ex)\n");
        fprintf(fcsx,"\t\t\t{\n");
        fprintf(fcsx,"\t\t\t}\n");


        fprintf(fcsx,"\t\t}\n");
        fprintf(fcsx,"\t\tselectedView.Initialised = true;\n");
        fprintf(fcsx,"\t}\n");

        fprintf(fcsx,"\tprivate void setViewToStandard()\n");
        fprintf(fcsx,"\t{\n");
        fprintf(fcsx,"\t\ttry\n");
        fprintf(fcsx,"\t\t{\n");
        fprintf(fcsx,"\t\t\tselectedView = new UserView();\n");
        fprintf(fcsx,"\t\t\tselectedView.Description = \"Standard\";\n");
        fprintf(fcsx,"\t\t\tselectedView.FormTypeID = this.FormTypeID;\n");
        fprintf(fcsx,"\t\t\tselectedView.Fields = appglobal.getDefaultViewFields(FormTypeID);\n");
        fprintf(fcsx,"\t\t\tUserViewBL b = new UserViewBL();\n");
        fprintf(fcsx,"\t\t\tgetFieldSettings(0);\n");
        fprintf(fcsx,"\t\t\tselectedView = b.mashFieldsWithSettings(selectedView);\n");
        fprintf(fcsx,"\t\t}\n");
        fprintf(fcsx,"\t\tcatch (Exception ex)\n");
        fprintf(fcsx,"\t\t{\n");
        fprintf(fcsx,"\t\t\tErrorLogBL.addLog(appglobal.curUserID, \"ERROR\", \"FXXX-0XX\", $\"V={appglobal.appVersion} U={appglobal.curUserName} {ex.Message}\");\n");
        fprintf(fcsx,"\t\t\tMessageBox.Show(\"ERROR FXXX-0XX :\" + ex.Message);\n");
        fprintf(fcsx,"\t\t}\n");
        fprintf(fcsx,"\t}\n");












        fprintf(fcsx,"\tprivate void doSearch(string srchString)\n");
        fprintf(fcsx,"\t{\n");
        fprintf(fcsx,"\t\t%sBL b = new %sBL();\n",argv[1],argv[1]);
        fprintf(fcsx,"\t\tDataTable table;\n");
        fprintf(fcsx,"\t\tif (selectedView != null)\n");
        fprintf(fcsx,"\t\t{\n");
        fprintf(fcsx,"\t\ttable = b.get%ssDT(srchString, selectedView.Fields",argv[1]);
        if(hasBIN==1){
            fprintf(fcs,",appglobal.Key");
        }
        fprintf(fcs,");\n");
        fprintf(fcsx,"\t\t}\n");
        fprintf(fcsx,"\t\telse\n");
        fprintf(fcsx,"\t\t{\n");
        fprintf(fcsx,"\t\t\tstring strDefault = \"Description, Column2\";\n");
        fprintf(fcsx,"\t\t\ttable = b.get%ssDT(srchString, strDefault",argv[1]);
        if(hasBIN==1){
            fprintf(fcs,",appglobal.Key");
        }
        fprintf(fcs,");\n");
        fprintf(fcsx,"\t\t}\n");
        fprintf(fcsx,"\t\tdataGridView1.DataSource = table;\n");
        fprintf(fcsx,"\t\tdataGridView1.AutoResizeColumns();\n");
        fprintf(fcsx,"\t\tif (selectedView == null) { \n");
        fprintf(fcsx,"\t\t\tdataGridView1.Columns[\"Column2\"].AutoSizeMode = DataGridViewAutoSizeColumnMode.Fill;\n");
        fprintf(fcsx,"\t\t}\n");
        fprintf(fcsx,"\t\telse\n");
        fprintf(fcsx,"\t\t{\n");
        fprintf(fcsx,"\t\t\tif (selectedView.Initialised == false)\n");
        fprintf(fcsx,"\t\t\t{\n");
        fprintf(fcsx,"\t\t\tformatViewTable();\n");
        fprintf(fcsx,"\t\t\t}\n");
        fprintf(fcsx,"\t\t}\n");
        fprintf(fcsx,"\t\tdataGridView1.Columns[\"%sID\"].Visible = false;\n",argv[1]);
        fprintf(fcsx,"\t}\n");
        fprintf(fcsx,"\tprivate void getFieldSettings(int ViewID)\n");
        fprintf(fcsx,"\t{\n");
        fprintf(fcsx,"\t\tUserViewFieldUserSettingBL fb = new UserViewFieldUserSettingBL();\n");
        fprintf(fcsx,"\t\tselectedView.FieldSettings = fb.getUserViewFieldUserSettingsL(this.FormTypeID, appglobal.curUserID, ViewID);\n");
        fprintf(fcsx,"\t}\n");


        fprintf(fcsx,"\tprivate void UseThisForDataTableSave(){\n");
        fprintf(fcsx,"\t\tdataGridView%s.EndEdit()\n",argv[1]);
        fprintf(fcsx,"\t\tdataGridView%s.ClearSelection();\n",argv[1]);
        fprintf(fcsx,"\t\t%sBS.ResetBindings(true);\n",argv[1]);

        fprintf(fcsx,"\t\t%sBL b = new %sBL();\n",argv[1],argv[1]);
        fprintf(fcsx,"\t\t%s b = new %s();\n",argv[1],argv[1]);
        fprintf(fcsx,"\t\to.%sID = BLLGlobal.getDataRowInt(row,\"%sID\",o.%sID);b.load(o.%s,o);// if saving existings\n",argv[1],argv[1],argv[1],argv[1]);
        fprintf(fcsx,"\t\to.%sID = 0;// if adding new\n",argv[1]);
        count = 0;
        compresult = 0;
        tp = 2; // 0 = int, 1 = float, 2 = string, 3=decimal, 4=date
        for(count==0;count<totRows;count++){
            tp = 2; // 0 = int, 1 = float, 2 = string, 3=decimal, 4=date
            strcpy(teststring,"INT");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            }        
            strcpy(teststring,"TINYINT");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            }        
            strcpy(teststring,"int");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=0;
            } 

            strcpy(teststring,"DECIMAL(10,2)");
            compresult = strcmp(types[count], teststring);
            if(compresult==0){
                tp=3;
            }
            if(tp==0){
                /// number
                fprintf(fcsx,"\t\to.%s = BLLGlobal.getDataRowInt(row,\"%s\",o.%s);\n",fields[count],fields[count],fields[count]);
                
            }else if(tp==3){
                fprintf(fcsx,"\t\to.%s = BLLGlobal.getDataRowDouble(row,\"%s\",o.%s);\n",fields[count],fields[count],fields[count]);

            }else{
                fprintf(fcsx,"\t\to.%s = BLLGlobal.getDataRowString(row,\"%s\",o.%s);\n",fields[count],fields[count],fields[count]);
            }
                
        }

        fprintf(fcsx,"\t\t\n");
        fprintf(fcsx,"\t\t\n");
        fprintf(fcsx,"\t\t\n");

        fprintf(fcsx,"\t}\n");









        
        fclose(fcsx);
    }




    //// create the helper cs form procedures

    char filenamecsu[128];
    snprintf(filenamecsu, sizeof filenamecsu, "%s.unit.cs",argv[1]);


    FILE *fcsu;
    fcsu = fopen(filenamecsu,"w");

    count = 0;

    if(fcsu!=NULL){
        
        fprintf(fcsu,"namespace UnitTesting{\n\n");
        fprintf(fcsu,"\t[TestClass()]\n");
        fprintf(fcsu,"\tpublic class %sTests{\n",argv[1]);
        fprintf(fcsu,"\t\tprivate int testUserID = 37;\n");
        fprintf(fcsu,"\t\t[TestMethod()]\n");
        fprintf(fcsu,"\t\tpublic void TestLoad(){\n");
        fprintf(fcsu,"\t\t\tDALGlobal.connectionString = UTGlobal.connectionstring;\n");
        fprintf(fcsu,"\t\t\t%sBL b = new %sBL();\n",argv[1],argv[1]);
        fprintf(fcsu,"\t\t\t%s o = new %s();\n",argv[1],argv[1]);
        fprintf(fcsu,"\t\t\to = b.load(11111,o);\n");
        fprintf(fcsu,"\t\t\tif(o.LastError.Length>0){\n");
        fprintf(fcsu,"\t\t\t\tAssert.Fail(o.LastError);\n");
        fprintf(fcsu,"\t\t\t}else{\n");
        fprintf(fcsu,"\t\t\t\t\n");
        fprintf(fcsu,"\t\t\t}\n");
        fprintf(fcsu,"\t\t}\n");
        fprintf(fcsu,"\t\t[TestMethod()]\n");
        fprintf(fcsu,"\t\tpublic void TestSave(){\n");
        fprintf(fcsu,"\t\t\tDALGlobal.connectionString = UTGlobal.connectionstring;\n");
        fprintf(fcsu,"\t\t\t%sBL b = new %sBL();\n",argv[1],argv[1]);
        fprintf(fcsu,"\t\t\t%s o = new %s();\n",argv[1],argv[1]);
        fprintf(fcsu,"\t\t\t/// add some data?\n");
        fprintf(fcsu,"\t\t\to = b.load(11111,o);\n");
        fprintf(fcsu,"\t\t\tif(o.LastError.Length>0){\n");
        fprintf(fcsu,"\t\t\t\tAssert.Fail(o.LastError);\n");
        fprintf(fcsu,"\t\t\t}else{\n");
        fprintf(fcsu,"\t\t\t\t\n");
        fprintf(fcsu,"\t\t\t\to = b.save(o,testUserID);\n");
        fprintf(fcsu,"\t\t\t\tif(o.LastError.Length>0){\n");
        fprintf(fcsu,"\t\t\t\t\tAssert.Fail(o.LastError);\n");
        fprintf(fcsu,"\t\t\t\t}else{\n");
        fprintf(fcsu,"\t\t\t\t\t\n");
        fprintf(fcsu,"\t\t\t\t}\n");
        fprintf(fcsu,"\t\t\t}\n");
        fprintf(fcsu,"\t\t}\n");
        fprintf(fcsu,"\t\t[TestMethod()]\n");
        fprintf(fcsu,"\t\tpublic void TestDelete(){\n");
        fprintf(fcsu,"\t\t\tDALGlobal.connectionString = UTGlobal.connectionstring;\n");
        fprintf(fcsu,"\t\t\t%sBL b = new %sBL();\n",argv[1],argv[1]);
        fprintf(fcsu,"\t\t\t%s o = new %s();\n",argv[1],argv[1]);
        fprintf(fcsu,"\t\t\to = b.save(o,testUserID);\n");
        fprintf(fcsu,"\t\t\tif(o.LastError.Length>0){\n");
        fprintf(fcsu,"\t\t\t\tAssert.Fail(o.LastError);\n");
        fprintf(fcsu,"\t\t\t}else{\n");

        fprintf(fcsu,"\t\t\t\tif(o.%sID > 0){\n",argv[1]);
        fprintf(fcsu,"\t\t\t\t\tint i = b.delete(o,testUserID);\n");
        fprintf(fcsu,"\t\t\t\t\tif(i==0){\n");
        fprintf(fcsu,"\t\t\t\t\t\tAssert.Fail(\"save returned 0:\" + b.LastErrorB);\n");
        fprintf(fcsu,"\t\t\t\t\t}else{\n");
        fprintf(fcsu,"\t\t\t\t\t\tif(b.LastErrorB.Length>0){\n");
        fprintf(fcsu,"\t\t\t\t\t\t\tAssert.Fail(\"\" + b.LastErrorB);\n");
        fprintf(fcsu,"\t\t\t\t\t\t}\n");
        fprintf(fcsu,"\t\t\t\t\t}\n");
        fprintf(fcsu,"\t\t\t\t}else{\n");
        fprintf(fcsu,"\t\t\t\t\t\n");
        fprintf(fcsu,"\t\t\t\t}\n");
        fprintf(fcsu,"\t\t\t}\n");
        fprintf(fcsu,"\t\t\t\n");
        fprintf(fcsu,"\t\t}\n");
        
        fprintf(fcsu,"}\n");
        
        
        fclose(fcsu);
    }



/*



*/







exit(EXIT_SUCCESS);

}

