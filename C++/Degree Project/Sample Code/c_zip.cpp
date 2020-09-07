#pragma once
#include "c_zip.hpp"

#include "zlib.h"
#include <stdio.h>
#include <iostream>
#include <fstream>

unsigned long file_size(const char *filename)
{
	FILE *pFile = fopen(filename, "rb");
	fseek(pFile, 0, SEEK_END);
	unsigned long size = ftell(pFile);
	fclose(pFile);
	return size;
}

long GetFileSize(std::string filename)
{
	struct stat stat_buf;
	int rc = stat(filename.c_str(), &stat_buf);
	return rc == 0 ? stat_buf.st_size : -1;
}



int get_file_size(std::string filename) // path to file
{
	FILE *p_file = NULL;
	p_file = fopen(filename.c_str(), "rb");
	fseek(p_file, 0, SEEK_END);
	int size = ftell(p_file);
	fclose(p_file);
	return size;
}


int fileSize(const char *add) {
	std::ifstream mySource;
	mySource.open(add, std::ios_base::binary);
	mySource.seekg(0, std::ios_base::end);
	int size = mySource.tellg();
	mySource.close();
	return size;
}


int decompress_file(const char* infilename, const char* outfilename)
{
	//std::string infile;
	//std::string outfile;

	gzFile infile = gzopen(infilename, "rb");
	FILE *outfile = fopen(outfilename, "wb");
	if (!infile || !outfile) return -1;

	char buffer[128];
	int num_read = 0;
	while ((num_read = gzread(infile, buffer, sizeof(buffer))) > 0) {
		fwrite(buffer, 1, num_read, outfile);

	}

	gzclose(infile);
	fclose(outfile);
}
int compress_file(const char* infilename, const char* outfilename)
{

	//std::cout << "\n - compressing file( from = " << infilename << " ,  to = " << outfilename << " ) " << std::endl;
	//



	//
	FILE *infile   = fopen(infilename, "rb");
	gzFile outfile = gzopen(outfilename, "wb");
	if (!infile || !outfile) return -1;

	char inbuffer[128];
	int num_read = 0;
	unsigned long total_read = 0, total_wrote = 0;
	while ((num_read = fread(inbuffer, 1, sizeof(inbuffer), infile)) > 0) {
		total_read += num_read;
		gzwrite(outfile, inbuffer, num_read);

	}
	fclose(infile);
	gzclose(outfile);

}