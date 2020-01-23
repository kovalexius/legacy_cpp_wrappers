#include <fstream>
#include <cstdint>
#include <iostream>
#include <chrono>

#include <conio.h>

#include "../jShot/src/GDIScreenShooter.h"
#include "../jShot/src/DXScreenShooter.h"
#include "../jShot/src/DDrawScreenShooter.h"

const WORD g_B = static_cast<WORD>('B');
const WORD g_M = static_cast<WORD>('M');
const WORD g_BM = g_B + (g_M << 8);


void writeBmpFile(const std::vector<char>& _header, const std::vector<char>& _data)
{
    std::fstream outFile("snapshot.bmp", std::ios::binary | std::ios::out | std::ios::trunc);
    outFile.write(_header.data(), _header.size());
    outFile.write(_data.data(), _data.size());
}

void makeBmp(const CRectangle& _region)
{
	std::vector<char> header(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
	BITMAPFILEHEADER* pFHeader = reinterpret_cast<BITMAPFILEHEADER*>(header.data());
	BITMAPINFOHEADER* pInfoHeader = reinterpret_cast<BITMAPINFOHEADER*>(header.data() + sizeof(BITMAPFILEHEADER));

	CGDIScreenShooter scrShooter;
	auto body = scrShooter.getScreenshot(_region, *pInfoHeader);

	pFHeader->bfType = g_BM;
	pFHeader->bfSize = pInfoHeader->biSizeImage + header.size();
	pFHeader->bfReserved1 = 0;
	pFHeader->bfReserved2 = 0;
	pFHeader->bfOffBits = header.size();
	writeBmpFile(header, body);
}

void testPerfomance(const CRectangle& _region)
{
	CGDIScreenShooter scrShooter;

	BITMAPINFOHEADER tmp;
	uint64_t numIterations = 0;

	auto startTime = std::chrono::system_clock::now();
	for (numIterations = 0; _kbhit() == 0; numIterations++)
	{
		scrShooter.getScreenshot(_region, tmp);
	}
	auto endTime = std::chrono::system_clock::now();
	getch();

	std::chrono::duration<double> diffTime = endTime - startTime;
	std::cout << "Interval: \'" << diffTime.count() << "\' numIterations: \'" << numIterations << "\'" << std::endl;
}

void main()
{
    CRectangle region;
    region.m_leftBottomCorner.m_x = 0;
    region.m_leftBottomCorner.m_y = 0;
	region.m_size.m_x = 1920;
	region.m_size.m_y = 1080;

	//makeBmp(region);
	//testPerfomance(region);

	CDxScreenShooter dxScreenShooter;
	dxScreenShooter.GetScreenShot(region);

	CDDrawScreenShooter ddScreenShooter;
	ddScreenShooter.GetScreenShot(region);
}