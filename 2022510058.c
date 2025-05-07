
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlschemastypes.h>


////////////////////////////////////
//enum for alert Level
typedef enum _alertLevel{
    LOW = 1,
    MEDIUM,
    HIGH,
    CRITICAL,
    UNSIGNED
}AlertLevel;

//struct for JSON reading
typedef struct _JSON{
    char fileName[11];
    int keyEnd;
    int keyStart;
    char order[5];
}JSON;

//struct for reading from .csv
typedef struct _smartLog{
    char device_ID[8];
    char timestamp[19];
    float temperature;
    int humidity;
    char status[7];
    char location[31];
    AlertLevel level;
    int battery;
    char firmware_ver[7];
    int event_code;

}SmartLog;

//struct for offset 
typedef struct{
    char *field_name;
    size_t offset;
    size_t size;
} FieldInfo;
////////////////////////////////////




/////////////////////////////////////////////////////////////////////////
void help();
void whichSeperator(int choice, char *separator);
void delimeterCSV(char* line,char delimeter, char *tokens[]);
void readCSV(char *inputFile,char *outputFile, char delimeter, int opsys);
void clearLineEnd(char *line);
SmartLog tokensToStruct(char* tokens[]);
JSON JSONread();
void readBinary(char* input, SmartLog logs[]);
void createXML(SmartLog logs[],char * outputFile, char* order);
const char * enumDecode();
char* getLittleEndian(int value, char *littleEndian);
char* getBigEndian(int value, char *bigEndian);
uint32_t littleToInt(unsigned char *littleEndian);
int findFieldOffset(int keyStart, int keyEnd, FieldInfo fields[], int count);
void sortLogs(SmartLog logs[], int num_logs);
int compare_smartlogs(const void *a, const void *b);
void validateXML(char *xml, char *xsd);
/////////////////////////////////////////////////////////////////////////
//global variable for sorting according to field name
char global_field_name[31];
int main(int argc, char *argv[])
{
    /////////////////////////////////
    char inputFile[30]="";
    char outputFile[30]="";
    int separatorNum=0;
    int opsysNum=0;
    int conversionType;
    char separator;
    SmartLog logs[50];
    /////////////////////////////////

    //help menu
    if(argc ==2 && strcmp(argv[1],"-h") ==0){
        help();
    }
    else{


        for(int i=0; i<argc;i++){
            if (strcmp(argv[i], "-separator") == 0 && i + 1 < argc) {
            i+=1;
            
            separatorNum = atoi(argv[i]);
            
            
            

            } else if (strcmp(argv[i], "-opsys") == 0 && i + 1 < argc) {
            i+=1;
            opsysNum = atoi(argv[i]);
            
        
            }
        }   
        
        whichSeperator(separatorNum,&separator);
        
    
        //input and output given
        if(argc == 8 ){
            //1-input 2-output 3-conversion type
             strcpy(inputFile,argv[1]);
             strcpy(outputFile,argv[2]);
             conversionType=atoi(argv[3]);
             
            if(conversionType == 1){

                //read CSV to binary
                readCSV(inputFile,outputFile,separator,opsysNum);
            }
             
             else if(conversionType == 3){
                validateXML(inputFile, "2022510058.xsd");
             }
             
         }
     //json reading here 
        else if (argc == 7){
           strcpy(outputFile,argv[1]);
           conversionType=atoi(argv[2]);
           
            JSON myJSON = JSONread();
            //reading from binary to struct
            readBinary(myJSON.fileName,logs);
            //creating XML from structs array

            FieldInfo myFields[]={
                {"device_ID", offsetof(SmartLog, device_ID), sizeof(logs[0].device_ID)},
                {"timestamp", offsetof(SmartLog, timestamp), sizeof(logs[0].timestamp)},
                {"temperature", offsetof(SmartLog, temperature), sizeof(logs[0].temperature)},
                {"humidity", offsetof(SmartLog, humidity), sizeof(logs[0].humidity)},
                {"status", offsetof(SmartLog, status), sizeof(logs[0].status)},
                {"location", offsetof(SmartLog, location), sizeof(logs[0].location)},
                {"level", offsetof(SmartLog, level), sizeof(logs[0].level)},
                {"battery", offsetof(SmartLog, battery), sizeof(logs[0].battery)},
                {"firmware_ver", offsetof(SmartLog, firmware_ver), sizeof(logs[0].firmware_ver)},
                {"event_code", offsetof(SmartLog, event_code), sizeof(logs[0].event_code)}
            };
            int count = sizeof(myFields)/sizeof(myFields[0]);
            int fieldIndex = findFieldOffset(myJSON.keyStart, myJSON.keyEnd, myFields,count);
            printf("Sorting according to: %s\n",  myFields[fieldIndex].field_name);
            strcpy(global_field_name ,myFields[fieldIndex].field_name);
            sortLogs(logs, 50);

            createXML(logs,outputFile,myJSON.order);
            
            
        }

}
 
    return(0);
}

//-h
void help(){

    printf("< input_file >    = Source file to read from\n");
    printf("< output_file >    = Target file to write to (or the XSD file for validation)\n");
    printf("< conversion_type > :\n");
    printf("  1 -> CSV to Binary\n");
    printf("  2 -> Binary TO XML (reads binary file name from setupParams.json)\n");
    printf("  3-> XML validation with XSD\n");
    printf("-separator <P1>   = Required field separator (1=comma, 2=tab, 3=semicolon)\n");
    printf("-opsys <P2>       = Required line ending type (1=windows, 2=linux, 3=macos)\n");

}
//csv dosyasını okuma
void readCSV(char *inputFile,char *outputFile, char delimeter, int opsys){
    
    char *tokens[10];
    FILE *fp;
    fp = fopen(inputFile,"r");
    FILE *out;
    out = fopen(outputFile,"w");
    if(fp == NULL){
        puts("Error opening file!");
        exit(1);

    }
    char line[256];
    fgets(line,sizeof(line),fp);
    while(fgets(line,sizeof(line),fp) !=NULL){

        
        clearLineEnd(line);
        
        delimeterCSV(line,delimeter,tokens);
        
        
        
        
        SmartLog log = tokensToStruct(tokens);
        
        

        fwrite(&log,sizeof(SmartLog),1,out);



    }
    fclose(fp);
    fclose(out);
    


}
//seperator'a göre bölme
void delimeterCSV(char* line,char delimeter, char *tokens[]){
    
    int count =0;
    char *start = line;
    char *last;

    while((last = strchr(start,delimeter))!=NULL){
        if(start == last){

            tokens[count]="\0";
            count++;

        }
        else{
            *last='\0';
            tokens[count]=start;
            count++;
        }
        start=last+1;
    }

    if (*start == '\0') {
        tokens[count++] = "";  
    } else {
        tokens[count++] = start;
    }
    


}
//seperator belirleme
void whichSeperator(int choice, char *separator){
    if(choice == 1){
        *separator=',';
    }
    else if(choice ==2){
        *separator='\t';
    }
    else if(choice ==3){
        *separator=';';
    }
    else {
        puts("No such choice!");
        exit(1);
    }
}
//satır sonu temizleme
void clearLineEnd(char *line){
    
    line[strcspn(line, "\r\n")] = '\0';

} 
//tokens array'ını alır ve struct'a çevirir
SmartLog tokensToStruct(char* tokens[]){
    int value = -999;
    float valueFloat = -999.0f;
    SmartLog log;
    strcpy(log.device_ID, tokens[0]);
    strcpy(log.timestamp,tokens[1]);
    log.temperature = (tokens[2][0]=='\0') ? valueFloat: atof(tokens[2]);
    log.humidity = (tokens[3][0]=='\0') ? value: atoi(tokens[3]);
    strcpy(log.status,tokens[4]);
    strcpy(log.location,tokens[5]);

    if (strcmp(tokens[6], "LOW") == 0) log.level = LOW;
    else if (strcmp(tokens[6], "MEDIUM") == 0) log.level = MEDIUM;
    else if (strcmp(tokens[6], "HIGH") == 0) log.level = HIGH;
    else if (strcmp(tokens[6], "CRITICAL") == 0) log.level = CRITICAL;
    else log.level = UNSIGNED;

    log.battery = (tokens[7][0]=='\0') ? value: atoi(tokens[7]);
    strcpy(log.firmware_ver,tokens[8]);
    log.event_code = (tokens[9][0]=='\0') ? value: atoi(tokens[9]);

    return log;



}
//JSON dosyasını okuma
JSON JSONread(){
    FILE *fp;
    JSON json;
    char buffer[1024];
    struct json_object *parsed_json;
    struct json_object *fileName;
    struct json_object *keyStart;
    struct json_object *keyEnd;
    struct json_object  *order;

    fp = fopen("setupParams.json","r");
    fread(buffer,1024,1,fp);
    fclose(fp);

    parsed_json = json_tokener_parse(buffer);
    
    json_object_object_get_ex(parsed_json,"dataFileName", &fileName);
    json_object_object_get_ex(parsed_json,"keyStart", &keyStart);
    json_object_object_get_ex(parsed_json,"keyEnd", &keyEnd);
    json_object_object_get_ex(parsed_json,"order", &order);


    strcpy(json.fileName,json_object_get_string(fileName));
    
    json.keyEnd = json_object_get_int(keyEnd);
    json.keyStart = json_object_get_int(keyStart);
    strcpy(json.order,json_object_get_string(order));

    return json;
    
    




}
//binary dosyadan okuma
void readBinary(char* input, SmartLog logs[]){

    FILE *fp;
    SmartLog log;
    int i=0;
    fp =fopen(input,"r");
    
    if(fp ==NULL){
        puts("NO  SUCH FILE");
        exit(1);
    }
    while(fread(&log,sizeof(SmartLog),1,fp)==1){
        logs[i++] = log;
        

    }
    fclose(fp);


}
//xml yaratma
void createXML(SmartLog logs[], char * outputFile, char* order){

    xmlDocPtr doc = NULL;
    xmlNodePtr root_node=NULL;
    char buff[256];
    doc = xmlNewDoc(BAD_CAST "1.0");

    char rootname[20];
    strcpy(rootname, outputFile);
    char *p = strrchr(rootname, '.'); 
    if (p != NULL) {
        *p = '\0'; 
    }

    root_node = xmlNewNode(NULL, BAD_CAST rootname);


    xmlDocSetRootElement(doc,root_node);
    int counter=1;
    
    char littleEndian[9];
    char bigEndian[9];
    uint32_t num;
    //ascending xml yaratma
    if(strcmp(order,"ASC")==0){
        for(int i=0; i<50;i++){
        
            snprintf(buff, sizeof(buff), "%d", counter);
            xmlNodePtr entry= xmlNewChild(root_node,NULL, BAD_CAST "entry", NULL);
            xmlNewProp(entry, BAD_CAST "id", BAD_CAST buff );
            counter++;
            
            xmlNodePtr device = xmlNewChild(entry,NULL, BAD_CAST "device", NULL);
            xmlNodePtr metrics = xmlNewChild(entry,NULL, BAD_CAST "metrics", NULL);
            xmlNodePtr timestamp;
            if(strcmp(logs[i].timestamp, "\0")!=0){
                timestamp = xmlNewChild(entry,NULL, BAD_CAST "timestamp", logs[i].timestamp);
            }
            
            
            xmlNodePtr event_code;
            if(logs[i].event_code != -999){
                snprintf(buff, sizeof(buff), "%d", logs[i].event_code);
                event_code=xmlNewChild(entry,NULL, BAD_CAST "event_code",buff);
                xmlNewProp(event_code, BAD_CAST "hexBig", BAD_CAST getBigEndian(logs[i].event_code, bigEndian));
                xmlNewProp(event_code, BAD_CAST "hexLittle", BAD_CAST getLittleEndian(logs[i].event_code, littleEndian));
                uint32_t num = littleToInt((unsigned char*)littleEndian);
                snprintf(buff, sizeof(buff), "%d", num);
                xmlNewProp(event_code, BAD_CAST "decimal", BAD_CAST buff);
            }
    
            
            uint32_t num = littleToInt((unsigned char*)littleEndian);
            snprintf(buff, sizeof(buff), "%d", num); 
            

            if(strcmp(logs[i].device_ID, "\0")!=0){
                xmlNewChild(device,NULL, BAD_CAST "device_id", BAD_CAST logs[i].device_ID);
            }
            if(strcmp(logs[i].location, "\0")!=0){
                xmlNewChild(device,NULL, BAD_CAST "location", BAD_CAST logs[i].location);
            }
            
            if(strcmp(logs[i].firmware_ver, "\0")!=0){
                xmlNewChild(device,NULL, BAD_CAST "firmware_ver", BAD_CAST logs[i].firmware_ver);
            }
            

            
            if(logs[i].temperature != -999.0f){
                snprintf(buff, sizeof(buff), "%.1f", logs[i].temperature);
                xmlNewChild(metrics,NULL, BAD_CAST "temperature", BAD_CAST buff);
            }
            
            if(logs[i].humidity != -999){
                snprintf(buff, sizeof(buff), "%d", logs[i].humidity);
                xmlNewChild(metrics,NULL, BAD_CAST "humidity", BAD_CAST buff);
            }            
            
            if(logs[i].battery != -999){
                snprintf(buff, sizeof(buff), "%d", logs[i].battery);
                xmlNewChild(metrics,NULL, BAD_CAST "battery", BAD_CAST buff);
            }

               
            
            if(strcmp(logs[i].status, "\0")!=0){
                xmlNewProp(metrics, BAD_CAST "status", BAD_CAST logs[i].status);
            }
            if(enumDecode(logs[i]) != "UNSIGNED"){
                xmlNewProp(metrics, BAD_CAST "alert_level", BAD_CAST enumDecode(logs[i]));
            }
            
            
            

            memset(littleEndian, 0, sizeof(littleEndian));
        }
    }
    //descending xml yaratma
    else if(strcmp(order,"DESC")==0){
        for(int i=49; i>=0; i--){
            snprintf(buff, sizeof(buff), "%d", counter);
            xmlNodePtr entry= xmlNewChild(root_node,NULL, BAD_CAST "entry", NULL);
            xmlNewProp(entry, BAD_CAST "id", BAD_CAST buff );
            counter++;
            
            xmlNodePtr device = xmlNewChild(entry,NULL, BAD_CAST "device", NULL);
            xmlNodePtr metrics = xmlNewChild(entry,NULL, BAD_CAST "metrics", NULL);
            xmlNodePtr timestamp;
            if(strcmp(logs[i].timestamp, "\0")!=0){
                timestamp = xmlNewChild(entry,NULL, BAD_CAST "timestamp", logs[i].timestamp);
            }
            
            
            xmlNodePtr event_code;
            if(logs[i].event_code != -999){
                snprintf(buff, sizeof(buff), "%d", logs[i].event_code);
                event_code=xmlNewChild(entry,NULL, BAD_CAST "event_code",buff);
                xmlNewProp(event_code, BAD_CAST "hexBig", BAD_CAST getBigEndian(logs[i].event_code, bigEndian));
                xmlNewProp(event_code, BAD_CAST "hexLittle", BAD_CAST getLittleEndian(logs[i].event_code, littleEndian));
                uint32_t num = littleToInt((unsigned char*)littleEndian);
                snprintf(buff, sizeof(buff), "%d", num);
                xmlNewProp(event_code, BAD_CAST "decimal", BAD_CAST buff);
            }
    
            
            uint32_t num = littleToInt((unsigned char*)littleEndian);
            snprintf(buff, sizeof(buff), "%d", num); 
            

            if(strcmp(logs[i].device_ID, "\0")!=0){
                xmlNewChild(device,NULL, BAD_CAST "device_id", BAD_CAST logs[i].device_ID);
            }
            if(strcmp(logs[i].location, "\0")!=0){
                xmlNewChild(device,NULL, BAD_CAST "location", BAD_CAST logs[i].location);
            }
            
            if(strcmp(logs[i].firmware_ver, "\0")!=0){
                xmlNewChild(device,NULL, BAD_CAST "firmware_ver", BAD_CAST logs[i].firmware_ver);
            }
            

            
            if(logs[i].temperature != -999.0f){
                snprintf(buff, sizeof(buff), "%.1f", logs[i].temperature);
                xmlNewChild(metrics,NULL, BAD_CAST "temperature", BAD_CAST buff);
            }
            
            if(logs[i].humidity != -999){
                snprintf(buff, sizeof(buff), "%d", logs[i].humidity);
                xmlNewChild(metrics,NULL, BAD_CAST "humidity", BAD_CAST buff);
            }            
            
            if(logs[i].battery != -999){
                snprintf(buff, sizeof(buff), "%d", logs[i].battery);
                xmlNewChild(metrics,NULL, BAD_CAST "battery", BAD_CAST buff);
            }

               
            
            if(strcmp(logs[i].status, "\0")!=0){
                xmlNewProp(metrics, BAD_CAST "status", BAD_CAST logs[i].status);
            }
            if(enumDecode(logs[i]) != "UNSIGNED"){
                xmlNewProp(metrics, BAD_CAST "alert_level", BAD_CAST enumDecode(logs[i]));
            }
            
            
            

            memset(littleEndian, 0, sizeof(littleEndian));
        }
    }
    

    xmlSaveFormatFileEnc(outputFile, doc, "UTF-8", 1);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    xmlMemoryDump();
        
}

const char * enumDecode(SmartLog log){
    //enumu decode eder
    char *level;
    switch(log.level){
        case LOW:
            level = "LOW";
            break;
        case MEDIUM:
            level = "MEDIUM";
            break;
        case HIGH:
            level = "HIGH";
            break;
        case CRITICAL:
            level = "CRITICAL";
            break;
        default:
            level = "UNSIGNED";
            break;

    }
    return level;
}

void validateXML(char *xml, char *xsd){
    xmlDocPtr doc;
    xmlSchemaPtr schema = NULL;
    xmlSchemaParserCtxtPtr ctxt;
	
    char XMLFileName[20]; 
    char XSDFileName[20]; 

    strcpy(XMLFileName , xml);
    strcpy(XSDFileName , xsd);
    
    
    xmlLineNumbersDefault(1); //set line numbers, 0> no substitution, 1>substitution
    ctxt = xmlSchemaNewParserCtxt(XSDFileName); //create an xml schemas parse context
    schema = xmlSchemaParse(ctxt); //parse a schema definition resource and build an internal XML schema
    xmlSchemaFreeParserCtxt(ctxt); //free the resources associated to the schema parser context
    
    doc = xmlReadFile(XMLFileName, NULL, 0); //parse an XML file
    if (doc == NULL)
    {
        fprintf(stderr, "Could not parse %s\n", XMLFileName);
    }
    else
    {
        xmlSchemaValidCtxtPtr ctxt;  //structure xmlSchemaValidCtxt, not public by API
        int ret;
        
        ctxt = xmlSchemaNewValidCtxt(schema); //create an xml schemas validation context 
        ret = xmlSchemaValidateDoc(ctxt, doc); //validate a document tree in memory
        if (ret == 0) //validated
        {
            printf("%s validates\n", XMLFileName);
        }
        else if (ret > 0) //positive error code number
        {
            printf("%s fails to validate\n", XMLFileName);
        }
        else //internal or API error
        {
            printf("%s validation generated an internal error\n", XMLFileName);
        }
        xmlSchemaFreeValidCtxt(ctxt); //free the resources associated to the schema validation context
        xmlFreeDoc(doc);
    }
    // free the resource
    if(schema != NULL)
        xmlSchemaFree(schema); //deallocate a schema structure

    xmlSchemaCleanupTypes(); //cleanup the default xml schemas types library
    xmlCleanupParser(); //cleans memory allocated by the library itself 
    xmlMemoryDump(); //memory dump
    
}
    


char* getLittleEndian(int value, char* littleEndian){

    //unsigned char alıyoruz 0 255 arası
    uint32_t num = (uint32_t)value;
    unsigned char *p = (unsigned char *)&num;
    
    //byte'ı 2 hex'e ayırır bastan giderek
    for(int i =0; i<4; i++){
        sprintf(littleEndian + i*2, "%02X", p[i]);
    }
    littleEndian[8] = '\0';

    return littleEndian;

}
char* getBigEndian(int value, char* bigEndian){

    //unsigned char alıyoruz 0 255 arası 
    uint32_t num = (uint32_t)value;
    unsigned char *p = (unsigned char *)&num;
    
    //sondan giderek byte'i 2 hex'e ayırır
    for (int i = 0; i < 4; i++) {
        sprintf(bigEndian + i*2, "%02X", p[3-i]);
    }
    bigEndian[8] = '\0';

    return bigEndian;

}
uint32_t littleToInt(unsigned char *littleEndian){
    
    uint32_t num;
    //littleendianı al ve hex'e çevir ve num'a ata
    sscanf(littleEndian, "%x"   , &num);
    return num;


}
int findFieldOffset(int keyStart, int keyEnd, FieldInfo fields[], int count){
    //keyStart ve keyEnd'e göre hangi field'a ait olduğunu bul 
    for(int i=0; i< count;i++){
        size_t start_field = fields[i].offset;
        size_t end_field = fields[i].offset + fields[i].size-1;
        if(keyStart >= start_field && keyEnd <= end_field){
            return i;
        }
    }
    return -1;
}
int compare_smartlogs(const void *a, const void *b) {
    //negatif :> correct position a-b (a is first)
    //pozitif :> wrong position a-b (b is first)
    //0 :> equal
    const SmartLog *logA = (const SmartLog *)a;
    const SmartLog *logB = (const SmartLog *)b;

    if (strcmp(global_field_name, "device_ID") == 0) {
        return strcmp(logA->device_ID, logB->device_ID);
    }
    else if (strcmp(global_field_name, "timestamp") == 0) {
        return strcmp(logA->timestamp, logB->timestamp);
    }
    else if (strcmp(global_field_name, "temperature") == 0) {
        if (logA->temperature < logB->temperature) return -1;
        if (logA->temperature > logB->temperature) return 1;
        return 0;
    }
    else if (strcmp(global_field_name, "humidity") == 0) {
        return logA->humidity - logB->humidity;
    }
    else if (strcmp(global_field_name, "status") == 0) {
        return strcmp(logA->status, logB->status);
    }
    else if (strcmp(global_field_name, "location") == 0) {
        return strcmp(logA->location, logB->location);
    }
    else if (strcmp(global_field_name, "level") == 0) {
        return logA->level - logB->level;
    }
    else if (strcmp(global_field_name, "battery") == 0) {
        return logA->battery - logB->battery;
    }
    else if (strcmp(global_field_name, "firmware_ver") == 0) {
        return strcmp(logA->firmware_ver, logB->firmware_ver);
    }
    else if (strcmp(global_field_name, "event_code") == 0) {
        return logA->event_code - logB->event_code;
    }
    
    
    return 0;
}
void sortLogs(SmartLog logs[], int num_logs) {
    
    //dizi, kaç tane eleman var, dizinin her elemanın boyutu, neye göre sıralanacak
    qsort(logs, num_logs, sizeof(SmartLog), compare_smartlogs);
}
    
    
    

    

