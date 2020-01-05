#include <stdio.h>
#include <stdlib.h>
int main(void)
{
        char *data;
        char a[10],b[10];
        printf("Content-Type:text/html\n\n");
        printf("<HTML>\n");
        printf("<HEAD>\n<TITLE >Get</TITLE>\n</HEAD>\n");
        printf("<BODY>\n");
        printf("<div style=\"font-size:12px\">\n");
        data = getenv("QUERY_STRING");
        if(sscanf(data,"a=%[^&]&b=%s",a,b)!=2){
                printf("<DIV STYLE=\"COLOR:RED\">Error parameters should be entered!</DIV>\n");
        }
else{
               printf("<DIV STYLE=\"COLOR:RED; font-size:15px;font-weight:bold\">Result = %d</DIV>\n",atoi(a)+atoi(b));
        }
        printf("</div>\n");
        printf("</BODY>\n");
        printf("</HTML>\n");
        return 0;
}
