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
    //// step 2
    //// CREATE sql file

    char filename[128];
    snprintf(filename, sizeof filename, "%s.sql",argv[1]);


    FILE *fsql;
    fsql = fopen(filename,"w");

    int count = 0;


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
        }
        fprintf(fsql,"\tRecordDeleted\t\tINT DEFAULT 0,\n");
        fprintf(fsql,"\tRecordLockByUserID\tINT DEFAULT 0,\n");
        fprintf(fsql,"\tRecordLockTime\t\tDATETIME NULL,\n");
        fprintf(fsql,"\tdateCreated\t\tDATETIME DEFAULT CURRENT_TIMESTAMP,\n");
        fprintf(fsql,"\tdateModified\t\tDATETIME DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,\n");
        fprintf(fsql,"\tModifiedByUserID\tINT DEFAULT 0,\n");
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

        count = 0;
        fprintf(fsql,")\n");
        fprintf(fsql,"BEGIN\n");
        fprintf(fsql,"\tDECLARE v_count INT;\n\n");
        fprintf(fsql,"\tSET v_count=(SELECT count(%sID) FROM tbl%s where %sID=v%sID);\n",argv[1],argv[1],argv[1],argv[1]);
        fprintf(fsql,"\tIF(v_count > 0) THEN\n");
        fprintf(fsql,"\t\tUPDATE tbl%s SET\n",argv[1]);
        for(count==0;count<totRows;count++){
            if(count==totRows-1){
                //fprintf(fsql,"		%s=v%s\n",fields[count],fields[count]);	
                fprintf(fsql,"\t\t%s=v%s,\n",fields[count],fields[count]);	
            }else{
                fprintf(fsql,"\t\t%s=v%s,\n",fields[count],fields[count]);	
            }
        }
        fprintf(fsql,"\t\tModifiedByUserID=vCurrentUserID\n");	
        fprintf(fsql,"\t\tWHERE %sID = v%sID;\n",argv[1],argv[1]);
        fprintf(fsql,"\t\tselect v%sID as %sID;\n",argv[1],argv[1]);

        count=0;	
        fprintf(fsql,"\tELSE\n\n\t\tINSERT INTO tbl%s(\n",argv[1]);
        for(count==0;count<totRows;count++){
            if(count==totRows-1){
                //fprintf(fsql,"		%s\n",fields[count]);	
                fprintf(fsql,"\t\t\t%s,\n",fields[count]);	
            }else{
                fprintf(fsql,"\t\t\t%s,\n",fields[count]);	
            }
        }
        fprintf(fsql,"\t\t\tModifiedByUserID\n");	
        count = 0;
        fprintf(fsql,"\t\t) VALUES (\n");
        for(count==0;count<totRows;count++){
            if(count==totRows-1){
                //fprintf(fsql,"		v%s\n",fields[count]);	
                fprintf(fsql,"\t\t\tv%s,\n",fields[count]);	
            }else{
                fprintf(fsql,"\t\t\tv%s,\n",fields[count]);	
            }
        }
        fprintf(fsql,"\t\t\tvCurrentUserID \n");	
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
        fprintf(fsql,"BEGIN\n\tUPDATE tbl%s SET RecordLockedByUserID=vCurrentUserID, RecordLockTime=CURRENT_TIMESTAMP WHERE %sID=v%sID;\nEND\n",argv[1],argv[1],argv[1]);

        fprintf(fsql,"\n\n//\n\n");
        
        fprintf(fsql,"DROP PROCEDURE IF EXISTS sp_Unlock%sRecord;\n\n", argv[1]);
        fprintf(fsql,"CREATE PROCEDURE sp_Unlock%sRecord(\n	v%sID INT\n)\n", argv[1],argv[1]);
        fprintf(fsql,"BEGIN\n\tUPDATE tbl%s SET RecordLockedByUserID=0, RecordLockTime=NULL WHERE %sID=v%sID;\nEND\n",argv[1],argv[1],argv[1]);

        fprintf(fsql,"\n\n//\n\n");
        fprintf(fsql,"\n/**************************************** \n\n");
        fprintf(fsql,"\n     end  %s \n\n",argv[1]);
        fprintf(fsql,"\n****************************************/ \n\n");

        fclose(fsql);

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
            tp = 2; // 0 = int, 1 = float, 2 = string, 3=decimal, 4=date
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

        fprintf(fcs,"\tpublic %s load(int ID, %s o){\n",argv[1],argv[1]);
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


        fprintf(fcs,"\tpublic %s load(int ID, MySqlConnection con, %s o){\n", argv[1], argv[1]);
        fprintf(fcs,"\t\t_LastErrorD = \"\";\n");
        fprintf(fcs,"\t\to.%sID = ID;\n",argv[1]);
        fprintf(fcs,"\t\tstring sql = \"SELECT \";\n");
        fprintf(fcs,"\t\tsql += \"%sID \";\n", argv[1]);
        count = 0;
        for(count==0;count<totRows;count++){
            fprintf(fcs,"\t\tsql += \",%s \";\n", fields[count]);
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
        
        fprintf(fcs,"\tpublic %s save(%s o, int curUserID){\n",argv[1],argv[1]);
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
        fprintf(fcs,"\tpublic %s save(MySqlConnection con, %s o, int curUserID){\n",argv[1],argv[1]);
        fprintf(fcs,"\t\tint ret = 0;\n");
        fprintf(fcs,"\t\tusing(MySqlCommand cmd = new MySqlCommand(\"sp_Update%s\",con)){\n", argv[1]);
        fprintf(fcs,"\t\t\tcmd.CommandType = CommandType.StoredProcedure;\n");
        fprintf(fcs,"\t\t\tcmd.Parameters.AddWithValue(\"v%sID\",o.%sID);\n",argv[1],argv[1]);
        count = 0;
        for(count==0;count<totRows;count++){
            fprintf(fcs,"\t\t\tcmd.Parameters.AddWithValue(\"v%s\",o.%s);\n",fields[count],fields[count]);
            
        }
        fprintf(fcs,"\t\t\tcmd.Parameters.AddWithValue(\"vCurrentUserID\",curUserID);\n");
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

        fprintf(fcs,"\t\tList<%s> l = new List<%s>(); \"\";\n",argv[1],argv[1]);
        fprintf(fcs,"\t\t_LastErrorD = \"\";\n");
        fprintf(fcs,"\t\tstring sql = \"SELECT \";\n");
        fprintf(fcs,"\t\tsql += \"%sID \";\n", argv[1]);
        count = 0;
        for(count==0;count<totRows;count++){
            fprintf(fcs,"\t\tsql += \",%s \";\n", fields[count]);
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
       



        fprintf(fcs,"\tpublic DataTable get%ssDT(string srchString, string strSelect){\n",argv[1]);
        fprintf(fcs,"\t\tDataTable dt = new DataTable();\n");
        fprintf(fcs,"\t\tstring connString = DALGlobal.connectionString; \n");
        fprintf(fcs,"\t\tusing (MySqlConnection con = new MySqlConnection(connString)){\n");
        fprintf(fcs,"\t\t\tcon.Open();\n");
        fprintf(fcs,"\t\t\tdt = get%ssDT(con,srchString, strSelect);\n",argv[1]);
        fprintf(fcs,"\t\t\tcon.Close();\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn dt;\n");
        fprintf(fcs,"\t}\n");
                    
        fprintf(fcs,"\tprivate DataTable get%ssDT(MySqlConnection con,string srchString, string strSelect){\n", argv[1]);

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
        fprintf(fcs,"\t\t\tsql += \" WHERE Description LIKE '%%\" + srchString + \"%%'\"\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\t//sql += \"ORDER BY Description \"\n");
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
        fprintf(fcs,"}\n\n\n");





        fprintf(fcs,"public class %sBL{\n",argv[1]);
        
        fprintf(fcs,"\tprotected string _LastErrorB = \"\";\n");
        fprintf(fcs,"\n\tpublic string LastErrorB {get=>_LastErrorB; set=> _LastErrorB=value;}\n\n");

        fprintf(fcs,"\tpublic %s load(int ID,%s o){\n",argv[1],argv[1]);
        fprintf(fcs,"\t\to.%sID=ID; \n",argv[1]);
        fprintf(fcs,"\t\t%sD d = new %sD(); \n",argv[1],argv[1]);
        fprintf(fcs,"\t\to = d.load(ID,o); \n");
        fprintf(fcs,"\t\t_LastErrorB = d.LastErrorD; \n");
        fprintf(fcs,"\t\treturn o; \n");

        fprintf(fcs,"\t}\n\n");

        fprintf(fcs,"\tpublic %s save(%s o,int curUserID){\n",argv[1],argv[1]);
        fprintf(fcs,"\t\t_LastErrorB = \"\";\n");
        fprintf(fcs,"\t\t%sD d = new %sD(); \n",argv[1],argv[1]);
        fprintf(fcs,"\t\to = d.save(o,curUserID); \n");
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
        
        fprintf(fcs,"\tpublic DataTable get%ssDT(string srchString,string strSelect){\n",argv[1]);
        fprintf(fcs,"\t\t_LastErrorB = \"\";\n");
        fprintf(fcs,"\t\tDataTable dt = new DataTable();\n");
        fprintf(fcs,"\t\t%sD d = new %sD(); \n",argv[1],argv[1]);
        fprintf(fcs,"\t\tdt = d.get%ssDT(srchString, strSelect); \n",argv[1]);
        fprintf(fcs,"\t\t_LastErrorB = d.LastErrorD; \n");
        fprintf(fcs,"\t\treturn dt; \n");
        fprintf(fcs,"\t}\n\n");

        fprintf(fcs,"\tpublic DataTable get%ssDT(string srchString,int UserViewID){\n",argv[1]);
        fprintf(fcs,"\t\t_LastErrorB = \"\";\n");
        fprintf(fcs,"\t\tDataTable dt = new DataTable();\n");
        fprintf(fcs,"\t\t%sD d = new %sD(); \n",argv[1],argv[1]);
        fprintf(fcs,"\t\tif(UserViewID>0){\n");
        fprintf(fcs,"\t\t\tUserViewFieldBL uvbl = new UserViewFieldBL();; \n");
        fprintf(fcs,"\t\t\tList<UserViewField> o = new List<UserViewField>();\n");
        fprintf(fcs,"\t\t\to = uvbl.getUserViewFields(UserViewID);\n");
        fprintf(fcs,"\t\t\tif(o.Count > 0){\n");
        fprintf(fcs,"\t\t\t\tstring s = string.Join(\",\",o.Select(x => BLLGlobal.SafeFieldName(x.FieldName)));\n");
        fprintf(fcs,"\t\t\t\tdt = d.get%ssDT(srchString, s); \n",argv[1]);
        fprintf(fcs,"\t\t\t_LastErrorB = d.LastErrorD; \n");
        fprintf(fcs,"\t\t\t}else{\n");
        fprintf(fcs,"\t\t\t_LastErrorB = \"No Fields in this UserView\";\n");
        fprintf(fcs,"\t\t\t}\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn dt; \n");
        fprintf(fcs,"\t}\n\n");
        
        fprintf(fcs,"\tpublic DataTable get%ssDT(string srchString,List<UserViewField> fields){\n",argv[1]);
        fprintf(fcs,"\t\t_LastErrorB = \"\";\n");
        fprintf(fcs,"\t\tDataTable dt = new DataTable();\n");
        fprintf(fcs,"\t\t%sD d = new %sD(); \n",argv[1],argv[1]);
        fprintf(fcs,"\t\tif(fields.Count > 0){\n");
        fprintf(fcs,"\t\t\tstring s = string.Join(\",\",fields.Select(x => BLLGlobal.SafeFieldName(x.FieldName)));\n");
        fprintf(fcs,"\t\t\tdt = d.get%ssDT(srchString, s); \n",argv[1]);
        fprintf(fcs,"\t\t\t_LastErrorB = d.LastErrorD; \n");
        fprintf(fcs,"\t\t}else{\n");
        fprintf(fcs,"\t\t\t_LastErrorB = \"No Fields in this UserView\";\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn dt; \n");
        fprintf(fcs,"\t}\n\n");




















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

        fprintf(fcsx,"\t\tObj%s = %sbl.load%s(ID,Obj%s);\n",argv[1],argv[1],argv[1],argv[1]);
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
        fprintf(fcsx,"\t\t\t%s oOld = %sbl.load%s(Obj%s.%sID,oOld);\n",argv[1],argv[1],argv[1],argv[1],argv[1]);
        fprintf(fcsx,"\t\t\tif(oOld.%sID>0){\n",argv[1]);
        fprintf(fcsx,"\t\t\t%sbl.setHistory(cOld,%s,appglobal.curUserID);\n",argv[1],argv[1]);
        fprintf(fcsx,"\t\t\t}\n");
        fprintf(fcsx,"\t\t}\n");

        fprintf(fcsx,"\t\t%sbl.LastErrorB = \"\";\n",argv[1]);
        fprintf(fcsx,"\t\tObj%s = %sbl.save(Obj%s,appglobal.curUserID);\n",argv[1],argv[1],argv[1]);
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


        fprintf(fcsx,"\tprivate void loadViewsMenu(){\n");
        fprintf(fcsx,"\t\tviewToolStripMenuItem.DropDownItems.Clear();\n");
        fprintf(fcsx,"\t\tviewToolStripMenuItem.DropDownItems.Add(new ToolStripMenuItem(\"Standard\", null, ApplyView_Click,\"mnuVStandard\"));\n");
        fprintf(fcsx,"\t\tUserViewBL b = new UserViewBL();\n");
        fprintf(fcsx,"\t\tviews = b.getUserViewsL((int)Globals.FormViewTypes.Customer, appglobal.curUserID);\n");
        fprintf(fcsx,"\t\tforeach(UserView view in views){\n");
        fprintf(fcsx,"\t\t\tviewToolStripMenuItem.DropDownItems.Add(new ToolStripMenuItem(view.Description, null, ApplyView_Click, \"mnuV\" + view.UserViewID));\n");
        fprintf(fcsx,"\t\t}\n");
        fprintf(fcsx,"\tprivate void ApplyView_Click(object? sender, EventArgs e){\n");    
        fprintf(fcsx,"\t\tif (sender != null){\n");
        fprintf(fcsx,"\t\t\tToolStripItem i = (ToolStripItem)sender;\n");
        fprintf(fcsx,"\t\t\tif (i.Text == \"Standard\")\n");
        fprintf(fcsx,"\t\t\t\t{\n");
        fprintf(fcsx,"\t\t\t\t\tselectedView = null;\n");
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
        fprintf(fcsx,"\t\t}\n");
        fprintf(fcsx,"\t\tselectedView.Initialised = true;\n");
        fprintf(fcsx,"\t}\n");
        fprintf(fcsx,"\tprivate void doSearch(string srchString)\n");
        fprintf(fcsx,"\t{\n");
        fprintf(fcsx,"\t\t%sBL b = new %sBL();\n",argv[1],argv[1]);
        fprintf(fcsx,"\t\tDataTable table;\n");
        fprintf(fcsx,"\t\tif (selectedView != null)\n");
        fprintf(fcsx,"\t\t{\n");
        fprintf(fcsx,"\t\ttable = b.get%ssDT(srchString, selectedView.Fields);\n",argv[1]);
        fprintf(fcsx,"\t\t}\n");
        fprintf(fcsx,"\t\telse\n");
        fprintf(fcsx,"\t\t{\n");
        fprintf(fcsx,"\t\t\tstring strDefault = \"Description, Column2\";\n");
        fprintf(fcsx,"\t\t\ttable = b.get%ssDT(srchString, strDefault);\n",argv[1]);
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

