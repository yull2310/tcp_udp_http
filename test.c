#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "link_list.h"
#include "commfunc.h"

int convert_content_to_hex()
{
 	char arr[1024] = { 0 };
	unsigned char HexArr[1024] = { 0 };
	int len = 0;
	int i = 0;

	sprintf(arr, "%s", "01 02 03 04 05 A1 A2 A3 A4 B1 B2 B3 B4");
	HexDump(arr, strlen(arr));

	ConvertByteString2Hex(arr, HexArr, &len);
	FILE* pFile = fopen("./hex_data", "w");
	if(pFile) {
		fwrite(HexArr, 1, len, pFile);
	}

	printf("\n");
    return 0;
}


int main(int argc, char* argv[])
{
    ReadHexString("./test.log");
    
    return 0;
}
