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
        fprintf(fsql,"CREATE PROCEDURE sp_delete%s(\n	v%sID INT, vCurrentUserID\n)\n", argv[1],argv[1]);
        fprintf(fsql,"BEGIN\n\tUPDATE tbl%s SET RecordDeleted=1, ModifiedByUserID=vCurrentUserID WHERE %sID=v%sID;\nEND\n",argv[1],argv[1],argv[1]);

        fprintf(fsql,"\n\n//\n\n");
        
        fprintf(fsql,"DROP PROCEDURE IF EXISTS sp_Lock%sRecord;\n\n", argv[1]);
        fprintf(fsql,"CREATE PROCEDURE sp_Lock%sRecord(\n\tv%sID INT, vCurrentUserID\n)\n", argv[1],argv[1]);
        fprintf(fsql,"BEGIN\n\tUPDATE tbl%s SET RecordLockedByUserID=vCurrentUserID, RecordLockTime=CURRENT_TIMESTAMP WHERE %sID=v%sID;\nEND\n",argv[1],argv[1],argv[1]);

        fprintf(fsql,"\n\n//\n\n");
        
        fprintf(fsql,"DROP PROCEDURE IF EXISTS sp_Unlock%sRecord;\n\n", argv[1]);
        fprintf(fsql,"CREATE PROCEDURE sp_Unlock%sRecord(\n	v%sID INT\n)\n", argv[1],argv[1]);
        fprintf(fsql,"BEGIN\n\tUPDATE tbl%s SET RecordLockedByUserID=0, RecordLockTime=NULL WHERE %sID=v%sID;\nEND\n",argv[1],argv[1],argv[1]);

        fprintf(fsql,"\n\n//\n\n");

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
        
        fprintf(fcs,"\n\ninternal class %s{\n",argv[1]);

        fprintf(fcs,"\tprivate string _LastError;\n");
        fprintf(fcs,"\tprivate %sID = 0;\n",argv[1]);
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
                fprintf(fcs,"\tprivate string _%s;\n", fields[count]);
            }else if(tp==1){
                fprintf(fcs,"\tprivate int _%s;\n", fields[count]);
                
            }else if(tp==2){
                fprintf(fcs,"\tprivate float _%s;\n", fields[count]);
            }else if(tp==3){
                fprintf(fcs,"\tprivate double _%s;\n", fields[count]);
            }else if(tp==4){
                fprintf(fcs,"\tprivate DateTime _%s;\n", fields[count]);
            }else if(tp==5){
                fprintf(fcs,"\tprivate DateTime _%s;\n", fields[count]);
            }else{
                fprintf(fcs,"\tprivate int _%s;\n", fields[count]);
            }
        }
        fprintf(fcs,"\tprivate bool _RecordDeleted;\n");
        fprintf(fcs,"\tprivate int _RecordLockByUserID;\n");
        fprintf(fcs,"\tprivate DateTime _RecordLockTime;\n");
        fprintf(fcs,"\tprivate DateTime _dateCreated;\n");
        fprintf(fcs,"\tprivate DateTime _dateModified;\n");
        fprintf(fcs,"\tprivate int _ModifiedByUserID;\n");
        
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
                fprintf(fcs,"\tpublic string %s {get=>_%s; set=> _%s=value;}\n", fields[count], fields[count], fields[count]);
            }else if(tp==1){
                fprintf(fcs,"\tpublic int  %s {get=>_%s; set=> _%s=value;}\n", fields[count], fields[count], fields[count]);
                
            }else if(tp==2){
                fprintf(fcs,"\tpublic float %s {get=>_%s; set=> _%s=value;}\n", fields[count], fields[count], fields[count]);
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


        fprintf(fcs,"\tpublic bool load(ID){\n");
        fprintf(fcs,"\t\tbool ret = true;\n");
        fprintf(fcs,"\t\t_%sID = ID;\n",argv[1]);
        fprintf(fcs,"\t\tstring connString = new ConfigurationManager.ConnectionStrings[appglobal.connectionname].ToString();\n");
        fprintf(fcs,"\t\tusing (MySqlConnection con = new MySqlConnection(connString)){\n");
        fprintf(fcs,"\t\t\tcon.Open();\n");
        fprintf(fcs,"\t\t\tret = load(ID, con);\n");
        fprintf(fcs,"\t\t\tcon.Close();\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn ret;\n");
        fprintf(fcs,"\t}\n");


        fprintf(fcs,"\tpublic bool load(ID, MySqlConnection con){\n");
        fprintf(fcs,"\t\tbool ret = true;\n");
        fprintf(fcs,"\t\t_%sID = ID;\n",argv[1]);
        fprintf(fcs,"\t\tstring sql = \"SELECT \";\n");
        fprintf(fcs,"\t\tsql = \"%sID \";\n", argv[1]);
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
        fprintf(fcs,"\t\tsql += \"WHERE %sID=\" + _%sID + \";\";\n", argv[1],argv[1]);
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
                fprintf(fcs,"\t\t\t\t\t\t_%s =r.IsDBNull(\"%s\") ? \"\" : r.GetString(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else if(tp==1){
                fprintf(fcs,"\t\t\t\t\t\t_%s =r.IsDBNull(\"%s\") ? 0 : r.GetInt32(\"%s\");\n", fields[count], fields[count], fields[count]);
                
            }else if(tp==2){
                // float
                fprintf(fcs,"\t\t\t\t\t\t_%s =r.IsDBNull(\"%s\") ? 0 : r.GetInt32(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else if(tp==3){
                // double
                fprintf(fcs,"\t\t\t\t\t\t_%s =r.IsDBNull(\"%s\") ? 0 : r.GetInt32(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else if(tp==4){
                // DateTime
                fprintf(fcs,"\t\t\t\t\t\t_%s =r.IsDBNull(\"%s\") ? DateTime.MinValue : r.GetDateTime(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else if(tp==5){
                // Time
                fprintf(fcs,"\t\t\t\t\t\t_%s =r.IsDBNull(\"%s\") ? DateTime.MinValue : r.GetDateTime(\"%s\");\n", fields[count], fields[count], fields[count]);
            }else{
                fprintf(fcs,"\t\t\t\t\t\t_%s =r.IsDBNull(\"%s\") ? \"\" : r.GetString(\"%s\");\n", fields[count], fields[count], fields[count]);
            }
            
        }
        fprintf(fcs,"\t\t\t\t\t\t_RecordDeleted =r.IsDBNull(\"RecordDeleted\") ? 0 : r.GetInt32(\"RecordDeleted\");\n");
        fprintf(fcs,"\t\t\t\t\t\t_RecordLockByUserID =r.IsDBNull(\"RecordLockByUserID\") ? 0 : r.GetInt32(\"RecordLockByUserID\");\n");
        fprintf(fcs,"\t\t\t\t\t\t_RecordLockTime =r.IsDBNull(\"RecordLockTime\") ? DateTime.MinValue : r.GetDateTime(\"RecordLockTime\");\n");
        fprintf(fcs,"\t\t\t\t\t\t_dateCreated =r.IsDBNull(\"dateCreated\") ? DateTime.MinValue : r.GetDateTime(\"dateCreated\");\n");
        fprintf(fcs,"\t\t\t\t\t\t_dateModified =r.IsDBNull(\"dateModified\") ? DateTime.MinValue : r.GetDateTime(\"dateModified\");\n");
        fprintf(fcs,"\t\t\t\t\t\t_ModifiedByUserID =r.IsDBNull(\"ModifiedByUserID\") ? 0 : r.GetInt32(\"ModifiedByUserID\");\n");


        fprintf(fcs,"\t\t\t\t\t}\n");
        fprintf(fcs,"\t\t\t\t\tr.Close();\n");
        fprintf(fcs,"\t\t\t\t}\n");
        fprintf(fcs,"\t\t\t}\n");
        fprintf(fcs,"\t\t\t_LastError = \"\";\n");
        fprintf(fcs,"\t\t\tret = true;\n");
        fprintf(fcs,"\t\t}catch(Exception e){\n");
        fprintf(fcs,"\t\t\t_LastError = ex.Message;\n");
        fprintf(fcs,"\t\t\tret = false;\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn ret;\n");
        fprintf(fcs,"\t}\n");
        
        fprintf(fcs,"\tpublic int save(){\n");
        fprintf(fcs,"\t\tint ret = 0;\n");
        fprintf(fcs,"\t\tstring connString = new ConfigurationManager.ConnectionStrings[appglobal.connectionname].ToString();\n");
        fprintf(fcs,"\t\tusing (MySqlConnection con = new MySqlConnection(connString)){\n");
        fprintf(fcs,"\t\t\tcon.Open();\n");
        fprintf(fcs,"\t\t\tret = save(con);\n");
        fprintf(fcs,"\t\t\tcon.Close();\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn ret;\n");
        fprintf(fcs,"\t}\n");
        fprintf(fcs,"\n");
        fprintf(fcs,"\tpublic int save(){\n");
        fprintf(fcs,"\t\tint ret = 0;\n");
        fprintf(fcs,"\t\tusing(MySqlCommand cmd = new MySqlCommand(sp_Update%s,con)){\n", argv[1]);
        fprintf(fcs,"\t\t\tcmd.CommandType = CommandType.StoredProcedure;\n");
        fprintf(fcs,"\t\t\tcmd.Parameters.AddWithValue(\"v%sID\",this._%sID);\n",argv[1],argv[1]);
        count = 0;
        for(count==0;count<totRows;count++){
            fprintf(fcs,"\t\t\tcmd.Parameters.AddWithValue(\"v%s\",this._%s);\n",fields[count],fields[count]);
            
        }
        fprintf(fcs,"\t\t\tcmd.Parameters.AddWithValue(\"vModifiedByUserID\",appglobal.curUserID);\n");
        fprintf(fcs,"\t\t\ttry{\n");
        fprintf(fcs,"\t\t\t\tvar recid = cmd.ExecuteScalar();\n");
        fprintf(fcs,"\t\t\t\tint.TryParse(recid.ToString(), out ret);\n");
        fprintf(fcs,"\t\t\t}catch (Exception ex){\n");
        fprintf(fcs,"\t\t\t\tMessageBox.Show(ex.Message);\n");
        fprintf(fcs,"\t\t\t\tret = -1;\n");
        fprintf(fcs,"\t\t\t}\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\n");

        fprintf(fcs,"\t\treturn ret;\n");
        fprintf(fcs,"\t}\n");

        fprintf(fcs,"\tpublic int delete(){\n");
        fprintf(fcs,"\t\tint ret = 0;\n");
        fprintf(fcs,"\t\tstring connString = new ConfigurationManager.ConnectionStrings[appglobal.connectionname].ToString();\n");
        fprintf(fcs,"\t\tusing (MySqlConnection con = new MySqlConnection(connString)){\n");
        fprintf(fcs,"\t\t\tcon.Open();\n");
        fprintf(fcs,"\t\t\tret = delete(con);\n");
        fprintf(fcs,"\t\t\tcon.Close();\n");
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn ret;\n");
        fprintf(fcs,"\t}\n");

        fprintf(fcs,"\tpublic int delete(MySqlConnection con){\n");
        fprintf(fcs,"\t\tclsAuditLogItem a = new clsAuditLogItem(appglobal.curUserID, \"%s\", \"%sID\",this._%sID, clsAuditLog.ActionType.Delete, this._%sID, 0);\n",argv[1],argv[1],argv[1],argv[1]);
        fprintf(fcs,"\t\ta.save();\n");
        fprintf(fcs,"\t\tint ret = 0;\n");
        fprintf(fcs,"\t\tusing (MySqlCommand cmd = new MySqlCommand(\"sp_Delete%s\", con)){\n", argv[1]);
        fprintf(fcs,"\t\t\tcmd.CommandType = CommandType.StoredProcedure;\n");
        fprintf(fcs,"\t\t\tcmd.Parameters.AddWithValue(\"v%sID\",this._%sID);\n",argv[1],argv[1]);
        fprintf(fcs,"\t\t\tcmd.Parameters.AddWithValue(\"vModifiedByUserID\",appglobal.curUserID);\n");
        fprintf(fcs,"\t\t\ttry{\n");
        fprintf(fcs,"\t\t\t\tvar recid = cmd.ExecuteNonQuery();\n");
        fprintf(fcs,"\t\t\t\tint.TryParse(recid.ToString(), out ret);\n");
        fprintf(fcs,"\t\t\t}catch (Exception ex){\n");
        fprintf(fcs,"\t\t\t\tMessageBox.Show(ex.Message);\n");
        fprintf(fcs,"\t\t\t\tret = -1;\n");
        fprintf(fcs,"\t\t\t}\n");
        
        fprintf(fcs,"\t\t}\n");
        fprintf(fcs,"\t\treturn ret;\n");
        fprintf(fcs,"\t}\n\n");
                    
        
        fprintf(fcs,"\tpublic void setHistory(cls%s Old){\n",argv[1]);
        fprintf(fcs,"\t\tclsAuditLog a = new clsAuditLog();\n");
        fprintf(fcs,"\t\ta.DoHistory(this, Old, this._%sID);\n",argv[1]);
        fprintf(fcs,"\t}\n\n");
        fprintf(fcs,"\tpublic void setHistory(){\n");
        fprintf(fcs,"\t\tclsAuditLogItem a = new clsAuditLogItem(appglobal.curUserID, \"%s\", \"%sID\",this._%sID, clsAuditLog.ActionType.Insert, this._%sID, 0);\n",argv[1],argv[1],argv[1],argv[1]);
        fprintf(fcs,"\t\ta.save();\n");
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
        fprintf(fcsx,"\tcls%s %s;\n\n\n",argv[1],argv[1]);

        fprintf(fcsx,"\tpublic frm%s(){\n",argv[1]);
        fprintf(fcsx,"\t\tInitializeComponent();\n");
        fprintf(fcsx,"\t\tappglobal.applyTheme(this);\n");
        fprintf(fcsx,"\t}\n");
        fprintf(fcsx,"\tpublic event EventHandler<CloseFormEventArgs> SaveComplete;\n\n");
        fprintf(fcsx,"\tpublic void load%s(int ID){\n",argv[1]);
        fprintf(fcsx,"\t\tthis.%sID=ID;\n",argv[1]);
        fprintf(fcsx,"\t\tthis.%s = new cls%s();\n",argv[1],argv[1]);
        fprintf(fcsx,"\t\tif (%s.load(ID)){\n",argv[1]);
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
                fprintf(fcsx,"\t\t\t\tthis.txt%s.Text = %s.%s.ToString(\"MM/dd/yyyy\");\n", fields[count],argv[1],fields[count]);
                fprintf(fcsx,"\t\t\t}\n");
            }else{
                fprintf(fcsx,"\t\t\tthis.txt%s.Text = %s.%s;\n", fields[count],argv[1],fields[count]);
            }
                
        }
        
        fprintf(fcsx,"\t\t}else{\n");
        fprintf(fcsx,"\t\t\tMessageBox.Show(\"Unable to load record: \" + %s.LastError);\n",argv[1]);
        fprintf(fcsx,"\t\t}\n");
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
                fprintf(fcsx,"\t\tDateTime.TryParse(this.txt%s.Text + "", out %s.%s);\n",fields[count],argv[1],fields[count]);
            }else{
                fprintf(fcsx,"\t\t%s.%s = this.txt%s.Text;\n", argv[1],fields[count],fields[count]);
            }
                
        }
        fprintf(fcsx,"\t\tbool wasinsert = true;\n");
        fprintf(fcsx,"\t\tif (%s.%sID > 0){\n",argv[1],argv[1]);
        fprintf(fcsx,"\t\t\twasinsert = false;   \n");
        fprintf(fcsx,"\t\t\tcls%s cOld = new cls%s();\n",argv[1],argv[1]);
        fprintf(fcsx,"\t\t\tif (cOld.load(this.ContactID)){\n");
        fprintf(fcsx,"\t\t\t\t%s.setHistory(cOld);\n",argv[1]);
        fprintf(fcsx,"\t\t\t}\n");
        fprintf(fcsx,"\t\t}\n");
        
            
        fprintf(fcsx,"\t\tcontact.save();   \n");
        fprintf(fcsx,"\t\tif (wasinsert){\n\t\t\tcontact.setHistory();\n\t\t}\n");
        fprintf(fcsx,"\t\tEventHandler<CloseFormEventArgs> handler = this.SaveComplete;\n");
        fprintf(fcsx,"\t\tif(handler != null){\n");
        fprintf(fcsx,"\t\t\t/// this would signify that the parent form needs a refresh   \n");
        fprintf(fcsx,"\t\t\tCloseFormEventArgs e = new CloseFormEventArgs();\n");
        fprintf(fcsx,"\t\t\tif (wasinsert){\n");
        fprintf(fcsx,"\t\t\t\te.ComletedAction = CloseFormEventArgs.FormAction.RecordSavedInsert;\n");
        fprintf(fcsx,"\t\t\t}else{\n");
        fprintf(fcsx,"\t\t\t\te.ComletedAction = CloseFormEventArgs.FormAction.RecordSavedUpdate;\n");
        fprintf(fcsx,"\t\t\t}\n");
        fprintf(fcsx,"\t\t\thandler(this, e);\n");
        fprintf(fcsx,"\t\t}\n");
        fprintf(fcsx,"\n");
        fprintf(fcsx,"\t}\n");
        fclose(fcsx);
    }



















exit(EXIT_SUCCESS);

}

