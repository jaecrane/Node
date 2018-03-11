# Node

#include <stdio.h>

#include <string.h>

#include <time.h>

#include <pthread.h>

#include <stdlib.h>

 

struct xlInfo

{

	char xl_id[30];  // 가상머신 아이디.

	char xl_name[30];  // 가상머신 이름.

	char name[30];  // 가상머신 이름.

	char id[30];  // 가상머신 아이디 .

	char state[30];  // 가상머신 상태.

	char cpu_sec[30];  // 가상머신 씨피유 작동 시간(초).

	char cpu_per[30];  // 가상머신 씨피유 작동 시간(퍼센트).

	char mem_k[30];  // 가상머신 메모리 용량(kybte).

	char mem_per[30]; //  가상머신 메로리 사용량(퍼센트).

	char max_mem_k[30]; // 최대 매모리 사용량(kybe,사용안함).

	char max_mem_per[30]; // 최대 메모리 사용량(퍼센트, 사용안함).

	char vcpus[30];  //가상 씨피유 갯수.

	char nets[30];  // 사용안함.

	int nettx_k;  //  전송패킷 양.

	int netrx_k;  //  송신패킷 양.

}xlresult[10];  //  가상머신 정보 구조체.

 

struct timeinfo 

{

	char year[5];  // 년.

	char month[5]; // 월.

	char time[15]; // 시간.

	char date[5];  // 날짜.

	char aa[5]; // 사용안함.

	char bb[5]; //  사용안함.

	char alltimeinfo[50]; // 로그 파일 이름, "아이피_년_월_날짜_시간".

}timeinfo; // 로그 파일 정보 구조체.

 

struct hardinfo 

{
	int cpumhz; //  씨피유 헤르츠.
	int cpucore; //  씨피유 코어 갯수.
	int cpuusage; //  씨피유 사용량(%).
	int memoryall; //  메모리 총 용량.
	int memoryusage; // 메모리 사용 정도.
	int diskall; // 디스크 총 용량.
	int diskusage; // 디스크 사용정도. 
	int netrxall; // 받은 패킷양.
	int nettxall; // 보낸 패킷양.

}hardinfo;

 

int rm_count = 0; // 로그 파일 조절 변수 (100일시 로그 파일은 지워짐).

char ipid[16]; // 실제 파싱한 아이피 문자배열.

char tempc[16]; // 임시 아이피 문자배열.

int conn = 0;  //  인덱스 조절 변수.

char isright[20]; // xl list 와  xentop 의 정보에 대하여 필요한 정보인지 아닌지 판별하는 문자배열.

int activeVM = 0; // 가상머신의 수.

int tempVM = 0;  // 실시간으로 초기화되는 가상머신의 정보를 받아오기 위한 임시.

FILE *openxentop;  // xentop 명령어 파일 포인터.
int sysCC = 0; // system cpu count;
 

void makefile();  // 파일을 만듬.

void getxentop();  // 가상머신 정보 생성.

void getxllist();  // 가상머신 정보 생성.

void gethardinfo(); // 물리머신 정보 생성

void makeallinfo(); //가상머신 정보 총합.

 

 

int main()

{
	printf("start\n");
	openxentop = popen("xentop -b d 1", "r"); // xentop 모듈 가동.
	FILE *getcpuu = popen("cat /proc/cpuinfo | grep processor | wc -l", "r");  // 사용중인 씨피유 정도.
	char aaa[20];
	fgets(aaa,"%s", getcpuu);
	sysCC = atoi(aaa);
	pclose(getcpuu);

	while (1) {

// 파일만들기 -> xl list 로 가상머신의 이름, 아이디 가져오기 -> 가상머신 비활성화 탐지 -> xentop으로 가상머신 정보 가져오기 -> 물리 머신 정보 가져오기-> 정보총합 및 서버로 로그파일 전달.

		makefile();

		getxllist();

		if (activeVM == 0) {  //  가상머신이 한개도 활성화 안된 상태 탐지.

			sleep(3);

			continue;

			}

		getxentop();

		gethardinfo();

		makeallinfo();
	
		system("./siftp 163.152.111.78 3999");  // 엣지서버에 파일을 ftp를 통하여 전송함.

		system("rm 1*");

		//sleep(3); 	// 3초마다 가상머신 정보를 로그파일에 저장함.

		}

	return 0;

}

 

void makefile() {

	FILE *makedir = popen("mkdir ./chalog", "r"); // ./chalog, 가상머신 로그 저장 경로.

	pclose(makedir);

	FILE *getip = popen("ifconfig -a | grep \"inet \" | grep \"Bcast:\" | awk '{print $2}' | awk -F: '{print $2}' > xenhostip.txt", "r"); //물리머신의 아이피생성 후 xenhostip.txt에 저장.

	pclose(getip);

	FILE *file_getip = fopen("xenhostip.txt", "r");  // 아이피 파일에서 파일 포인터를 가져옴.

	memset(ipid, '_', sizeof(char) * 16); //  ipid 테이블 초기화

	memset(tempc, '_', sizeof(char) * 16);

	// 실제 아이피 정보 문자 배열 초기화

	fgets(tempc, 16, file_getip);

	for (conn = 0; conn<16; conn++) { // 임시 문자 배열에 저장 후, 쓰레기값 제거

		if (tempc[conn] == '_' || tempc[conn] == '\n') {

			for (; conn<16; conn++) {

				ipid[conn] = '\0';

			}

			break;

		}else {

			ipid[conn] = tempc[conn];

		}

	}

	fclose(file_getip);

	

	char filename[30];  // 로그 파일 이름 문자배열.

	char temp_filename[30];  // 임시 로그 파일 이름 문자배열.

	FILE *createname = popen("date > createDate.txt", "r");

	pclose(createname);

	FILE *file_filename = fopen("createDate.txt", "r");

	

	fgets(temp_filename, 30, file_filename);

	sscanf(temp_filename, "%s %s %s %s %s %s", timeinfo.aa, timeinfo.month, timeinfo.date, timeinfo.time, timeinfo.bb, timeinfo.year);  // 시스템 시간 정보 변수에 저장.

	fclose(file_filename);

	conn = 0;    // 시간정보를 구분하고 서버단에서 파싱하기 위해  ':' -> '_'로 바꿈.

	for (conn = 0; conn < 12; conn++) {

		if (timeinfo.time[conn] == ':') {

			timeinfo.time[conn] = '_';

		}

	}

// 모든 시간 정보및 아이피 주소를 가상머신 ./chalog디렉토리에 로그파일 이름으로 저장, "아이피_년_월_날짜_시간"

	memset(timeinfo.alltimeinfo, 0, sizeof(char) * 50);

	

	strcat(timeinfo.alltimeinfo, "./chalog/");

	strcat(timeinfo.alltimeinfo, ipid);

	strcat(timeinfo.alltimeinfo, "_");

	strcat(timeinfo.alltimeinfo, timeinfo.year);

	strcat(timeinfo.alltimeinfo, "_");

	strcat(timeinfo.alltimeinfo, timeinfo.month);

	strcat(timeinfo.alltimeinfo, "_");

	strcat(timeinfo.alltimeinfo, timeinfo.date);

	strcat(timeinfo.alltimeinfo, "_");

	strcat(timeinfo.alltimeinfo, timeinfo.time);

	strcat(timeinfo.alltimeinfo, ".txt"); //텍스트 파일로 저장.

}

 

void getxllist() {

 

	FILE *qos_fp = popen("xl list", "r"); //xl list 명령어 파일 포인터 생성

	char xlstr[256]; // xl list 명령어 결과값 저장 문자 배열

	activeVM = 0;

// qos_fp 명령어 파일 포인터로 가상머신 정보 추출.

	while (fgets(xlstr, 256, qos_fp) != NULL) {

		memset(isright, 0, sizeof(isright));

		sscanf(xlstr, "%s", isright);

	

		if (strcmp(isright, "Name") == 0) { continue; } // Name, Domain-0은 필요없는 정보이므로 배제함.

		if (strcmp(isright, "Domain-0") == 0) { continue; }

 

		sscanf(xlstr, "%s %s", xlresult[activeVM].xl_name, xlresult[activeVM].xl_id); // 가상머신 이름과 아이디 정보 추출함.

		activeVM++; // 가상머신의 갯수를 파악함.

	}

 

	if (activeVM == 0) { // 만약 activeVM의 갯수가 0이면 함수를 빠져나감.

		return;

	}else{

		pclose(qos_fp);

	}

}

 

void getxentop() {

	tempVM = activeVM; // 임시 가상머신 수에대한 변수, xentop의 명령어 결과는 실시간으로 초기화되어 가상머신의 수로 출력에 대하여 조절이 필요함.

	conn = 0;

	char topsrt[256]; //  xentop 명령어의 결과 저장 문자 배열.

//  main()구문에서 실행한 openxentop 명령어 파일 포인터를 이용해 정보 추출

	while (fgets(topsrt, 256, openxentop) != NULL) {

		if (tempVM == 0) { break; }

		memset(isright, 0, sizeof(isright));

		sscanf(topsrt, "%s", isright);

		if (strcmp(isright, "NAME") == 0) { continue; }  // 필요없는 정보를 배제함.

		if (strcmp(isright, "Domain-0") == 0) { continue; }

		// xentop 정보 저장.

		sscanf(topsrt, "%s %s %s %s %s %s %s %s %s %s %d %d \n", xlresult[conn].name, xlresult[conn].state, xlresult[conn].cpu_sec, xlresult[conn].cpu_per, xlresult[conn].mem_k, xlresult[conn].mem_per, xlresult[conn].max_mem_per, xlresult[conn].max_mem_per, xlresult[conn].vcpus, xlresult[conn].nets, &xlresult[conn].nettx_k, &xlresult[conn].netrx_k);

		conn++;

		tempVM--;  //tempVM의 값이 0이 되면 while구문을 빠져나감.

	}

}

 

void gethardinfo() {

	hardinfo.cpumhz = 0; // 물리머신 상태 변수 초기화.

	hardinfo.cpucore = 0;

	hardinfo.cpuusage = 0;

	hardinfo.memoryall = 0;

	hardinfo.memoryusage = 0;

	hardinfo.diskall = 0;

	hardinfo.diskusage = 0;

	hardinfo.nettxall = 0;

	hardinfo.netrxall = 0;

	char hardstr[256];

	int temp;

	FILE *getcpu = popen("xl info", "r"); //  xl info 명령어 파일 포인터 생성.

	while (fgets(hardstr, "256", getcpu) != NULL) {

		memset(isright, 0, sizeof(isright));

		sscanf(hardstr, "%s", isright);

		if (strcmp(isright, "cores_per_socket") == 0) {  // 코어 갯수 .

			sscanf(hardstr, "%s %s %d", xlresult[0].max_mem_k, xlresult[0].max_mem_per, &hardinfo.cpucore);

			continue;

		}

		if (strcmp(isright, "cpu_mhz") == 0) {  // 씨피유 헤르츠 .

			sscanf(hardstr, "%s %s %d", xlresult[0].max_mem_k, xlresult[0].max_mem_per, &hardinfo.cpumhz);

			continue;

		}

		if (strcmp(isright, "total_memory") == 0) {  // 전체 메모리 용량.

			sscanf(hardstr, "%s %s %d", xlresult[0].max_mem_k, xlresult[0].max_mem_per, &hardinfo.memoryall);

			continue;

		}

		if (strcmp(isright, "free_memory") == 0) {  //  사용중 메모리.

			sscanf(hardstr, "%s %s %d", xlresult[0].max_mem_k, xlresult[0].max_mem_per, &temp);

			hardinfo.memoryusage = hardinfo.memoryall - temp;

			break;

		}

	}

	pclose(getcpu);

 

	FILE *getdisk = popen("df --total", "r");  // df --total 파일 포인터 가져오기.

 

	while (fgets(hardstr, 256, getdisk) != NULL) {  // 전체 디스크 용량 및 사용중인 용량.

		memset(isright, 0, sizeof(isright));

		sscanf(hardstr, "%s", isright);

		if (strcmp(isright, "total") == 0) {

			sscanf(hardstr, "%s %d %d", xlresult[0].max_mem_k, &hardinfo.diskall, &hardinfo.diskusage);

		}

	}

	pclose(getdisk);

 

	for (conn = 0; conn<activeVM; conn++) {  // 트렉픽양(xlresult 구조체에서).

		hardinfo.netrxall += xlresult[conn].netrx_k;

		hardinfo.nettxall += xlresult[conn].nettx_k;

	}

 
		
	int cpuAmount = 0;
	for(conn=0; conn< activeVM; conn++){
		cpuAmount = cpuAmount +atoi(xlresult[conn].cpu_per);
	
	}
	cpuAmount = cpuAmount / sysCC;
	hardinfo.cpuusage = cpuAmount;

	

}

 

void makeallinfo() {

	int i = 0;

	int j = 0;

	for (i = 0; i < activeVM; i++) {                     // xl list에서 뽑은 결과 값들과 xentop에서 뽑은 결과 값들을 서로 매칭 시켜줌( 가상머신 이름으로)

		for (j = 0; j < activeVM; j++) {

			int result = strncmp(xlresult[i].xl_name, xlresult[j].name, 10);

			if (result == 0) {

				strcpy(xlresult[j].id, xlresult[i].xl_id);

				strcpy(xlresult[j].name, xlresult[i].xl_name);

			}

		}

	}

	FILE *allinfo = fopen(timeinfo.alltimeinfo, "w"); // 로그 파일에다가 각 값들을 저장함.

	conn = 0;

	for (conn = 0; conn < activeVM; conn++) {

		fprintf(allinfo, "%s %s %s %s %s %s %s %s %d %d \n", xlresult[conn].id, xlresult[conn].name, xlresult[conn].state, xlresult[conn].cpu_sec, xlresult[conn].cpu_per, xlresult[conn].mem_k, xlresult[conn].mem_per, xlresult[conn].vcpus, xlresult[conn].nettx_k, xlresult[conn].netrx_k);

		// "가상머신아이디, NAME, STATE, CPU(sec), CPU(%), MEM(k), MEM(%), MAXMEM(k), MAXMEM(%), VCPUS, NETTX(k), NETRX(k)".

	} 

	

	fprintf(allinfo, "%c %d %d %d %d %d %d %d %d %d", '$', hardinfo.cpumhz, hardinfo.cpucore, hardinfo.cpuusage, hardinfo.memoryall, hardinfo.memoryusage, hardinfo.diskall, hardinfo.diskusage, hardinfo.netrxall, hardinfo.nettxall);

	fclose(allinfo);
	  //  값이 100이 되면 ./chalog에 있는 로그파일 전체를 지움
		rm_count++;
		if (rm_count == 100) {

			FILE *rm_log = popen("rm -r ./chalog/*", "r");

			pclose(rm_log);

			rm_count = 0;

		}

	

}

