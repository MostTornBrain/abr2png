// abr.cpp : Defines the entry point for the console application.
//

#include <string>
#include <iostream>
#include <algorithm>
#include <cctype>

#include "abr_util.h"
#include "PngWrite.h"

using namespace std;

bool invert    = false;  // -i
bool png       = false;  // -png
bool pgm       = false;  // -pgm
int  png_level = 9;      // zlib compression level

bool find_key(string key,int argc, char* argv[]);

static std::string sanitize_filename(const std::string &in, const std::string &fallback)
{
	std::string out;
	out.reserve(in.size());

	// Characters invalid on Windows filenames: <>:"/\\|?* and control chars
	auto is_invalid = [](unsigned char c)->bool {
		if (c < 32) return true;
		switch (c) {
			case '<': case '>': case ':': case '"': case '/': case '\\':
			case '|': case '?': case '*': return true;
			default: return false;
		}
	};

	for (unsigned char c : in) {
		if (is_invalid(c))
			out.push_back('_');
		else
			out.push_back(c);
	}

	// Trim trailing spaces and dots (Windows forbids filenames that end with space or dot)
	while (!out.empty() && (out.back() == ' ' || out.back() == '.'))
		out.back() = '_';

	// If result is empty (or all replaced), fall back to the provided fallback
	if (out.empty())
		return fallback;

	return out;
}

void invert_brush(GimpBrush * brush)
{
	long data_size = brush->mask->height * brush->mask->width*brush->mask->bytes;

	for (int i = 0; i < data_size;i++)
		brush->mask->data[i]=255-brush->mask->data[i];
}

int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		cerr
			<<"Name:"<<endl
			<<"    abr2png - Photoshop Brush converter"<<endl
			<<endl
			<<"Version:"<<endl
			<<"    abr2png v2.0.1 (29-nov-2025)"<<endl
			<<endl
			<<"Description:"<<endl
			<<"    abr2png - tool to convert from Photoshop Brush (.abr) and"<<endl
			<<"              PaintShopPro (.jbr) to Portable Grayscale (.pgm)"<<endl
			<<"              or Portable Network Graphics (.png) formats"<<endl
			<<endl
			<<"Usage:"<<endl
			<<"    abr2png [options] [file.abr]"<<endl
			<<endl
			<<"Options:"<<endl
			<<"    -i    invert image"<<endl
			<<"    -png  output to png file format (default)"<<endl
			<<"    -pgm  output to pgm file format"<<endl
			<<"    -c0"<<endl
			<<"    ..."<<endl
			<<"    -c9   compression level. -c0 - no compression"<<endl
			<<"                             -c1 - best speed"<<endl
			<<"                             -c9 - best compression (default)"<<endl
			<<endl
			<<"Copyright (c) 2008, D.Gar'kaev aka Dickobraz (dickobraz@mail.ru)"<<endl
			<<"Modifications in 2025 by Brian Stormont (https://github.com/MostTornBrain/abr2png)"<<endl;
		return 1;
	}

	invert = find_key("-i", argc,argv);
	png = find_key("-png", argc,argv);
	pgm = find_key("-pgm", argc,argv);
	if (png == false && pgm == false) png = true;
	if (find_key("-c0",argc,argv)) png_level=0;
	if (find_key("-c1",argc,argv)) png_level=1;
	if (find_key("-c2",argc,argv)) png_level=2;
	if (find_key("-c3",argc,argv)) png_level=3;
	if (find_key("-c4",argc,argv)) png_level=4;
	if (find_key("-c5",argc,argv)) png_level=5;
	if (find_key("-c6",argc,argv)) png_level=6;
	if (find_key("-c7",argc,argv)) png_level=7;
	if (find_key("-c8",argc,argv)) png_level=8;
	if (find_key("-c9",argc,argv)) png_level=9;


	//brush_list_t::iterator list_iter;
	brush_map_t::iterator map_iter;
	
	char *file_name = argv[argc-1];
	//brush_list_t* b_l = NULL; // = brush_load_abr((const char *)file_name);
	brush_map_t* b_m = brush_load_abr((const char *)file_name);
	
	if (!b_m)
		return 1;
	GimpBrush *gbrush;
	int data_size;
	FILE *fout;
	std::string fname;
	char head[128]={0};
	int index=0;
	char s_index[128]={0};

    printf("BM size is %lu\n", b_m->size());
	
	for(map_iter = b_m->begin(); map_iter != b_m->end(); map_iter++) {
		gbrush=map_iter->second;
		printf("\tgbrush name=[%s] key=[%s]\n", gbrush->name, gbrush->key.c_str());
		data_size=gbrush->mask->height*gbrush->mask->width*gbrush->mask->bytes;
		sprintf(s_index,"%03d",index++);

		if(pgm) {
			fname=std::string((char *)file_name)+std::string("_")+std::string(s_index)+std::string(".pgm");
			fout=fopen(fname.c_str(),"wb");

			if(fout) {
				sprintf(head,"P5 %d %d 255\n",gbrush->mask->width,gbrush->mask->height);
				fwrite(head,strlen(head),1,fout);
				invert_brush(gbrush);
				fwrite(gbrush->mask->data,data_size,1,fout);
				fclose(fout);
			} else {
				std::cerr << "Failed to create PGM file '" << fname << "' for writing" << std::endl;
			}
		}

		if(png) {
			if (invert)
				invert_brush(gbrush);

			// Build a safe filename from the brush name; fall back to index if empty
			std::string raw_name = gbrush->name ? std::string((char *)gbrush->name) : std::string();
			std::string fallback = std::string("brush_") + std::string(s_index);
			std::string safe = sanitize_filename(raw_name, fallback);

			// Save 2 varicolor images: base and mask
			fname = safe + " vari_01" + std::string(".png");
			bool r = WritePNG(gbrush->mask->width, gbrush->mask->height, gbrush->mask->data, 4, VARICOLOR_BASE, png_level, fname.c_str());
			if (!r) {
				std::cerr << "Failed to write PNG file '" << fname << "'" << std::endl;
			}

			fname = safe + " vari_02" + std::string(".png");
			r = WritePNG(gbrush->mask->width, gbrush->mask->height, gbrush->mask->data, 4, VARICOLOR_MASK, png_level, fname.c_str());
			if (!r) {
				std::cerr << "Failed to write PNG file '" << fname << "'" << std::endl;
			}

		}

	}

	return 0;
}

bool find_key(string key, int argc, char* argv[])
{
	bool key_ok=false;
	for (int i = 0; i < argc; i++)
	{
		string par(argv[i]);
		if (par == key)
		{
			key_ok = true;
			break;
		}
	}
	return key_ok;
}
