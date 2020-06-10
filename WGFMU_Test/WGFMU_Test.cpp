#include <stdio.h>
#include <time.h>
#include "wgfmu.h"

void writeResults(int channelId, const char *fileName)
{
	FILE *fp;
	fopen_s(&fp, fileName, "a");
	if (fp != 0)
	{
		fprintf(fp, "#MeasureValue\n");
		int measuredSize, totalSize;
		WGFMU_getMeasureValueSize(channelId, &measuredSize, &totalSize);
		for (int i = 0; i < measuredSize; i++)
		{
			double time, value;
			WGFMU_getMeasureValue(channelId, i, &time, &value);
			fprintf(fp, "%.9lf, %.9lf\n", time, value);
		}
		fclose(fp);
	}
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		printf("参数错误\n");
		return 0;
	}

	time_t rawtime;
	char nowtime[20];
	struct tm ltime;

	int channel1 = 401;
	double vector[2] = { 0,0 };
	double measurementPoints = 0;

	char filepath[128];
	char filename1[7] = "\\Data_";
	char filename2[5] = ".csv";


	WGFMU_clear();
	WGFMU_createPattern("pulse", 0);

	FILE *fp = NULL;
	fopen_s(&fp, argv[1], "r");

	if (fp != 0) {
		while (fscanf_s(fp, "%lf", &vector[0]) != EOF) {
			if (fscanf_s(fp, "%lf", &vector[1]) == EOF) {
				printf("文件错误\n");
				return 0;
			}
			WGFMU_addVector("pulse", vector[0], vector[1]);
			measurementPoints += vector[0];
		}
		fclose(fp);
	}

	WGFMU_setMeasureEvent("pulse", "10nsSampling", 0, (int)(measurementPoints * 1e8), 1e-8, 0, WGFMU_MEASURE_EVENT_DATA_RAW);
	WGFMU_addSequence(channel1, "pulse", 1);

	time(&rawtime);
	localtime_s(&ltime, &rawtime);
	strftime(nowtime, 20, "%Y_%m_%d_%H_%M_%S", &ltime);
	sprintf_s(filepath, "%s%s%s%s", argv[2], filename1, nowtime, filename2);

	WGFMU_exportAscii(filepath);

	// ONLINE
	WGFMU_openSession("GPIB0::17::INSTR");
	WGFMU_initialize();
	WGFMU_setOperationMode(channel1, WGFMU_OPERATION_MODE_FASTIV);
	WGFMU_setMeasureMode(channel1, WGFMU_MEASURE_MODE_CURRENT);
	WGFMU_connect(channel1);
	WGFMU_execute();
	WGFMU_waitUntilCompleted();
	writeResults(channel1, filepath);
	WGFMU_initialize(); // WGFMU_disconnect();
	WGFMU_closeSession();
}