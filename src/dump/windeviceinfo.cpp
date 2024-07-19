#include <windows.h>
#include <tchar.h>
#include <setupapi.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <initguid.h>
#include <devpkey.h>
#include <Cfgmgr32.h>
#include <string>
#include <devguid.h>
#include <Usbiodef.h>
#include "hookapi.h"

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "Cfgmgr32.lib")



bool StartsWith(const std::wstring& str, const std::wstring& prefix)
{
	// 检查 str 的长度是否至少与 prefix 相同
	if (str.size() < prefix.size())
	{
		return false;
	}

	// 比较 str 的前缀部分是否等于 prefix
	return std::equal(prefix.begin(), prefix.end(), str.begin());
}


//void GetDeviceInterfaceProperty(const std::wstring& deviceInterface)
//{
//	DEVPROPKEY propertyKey = DEVPKEY_Device_LocationPaths; // 使用一个常见的属性键
//	DEVPROPTYPE propertyType;
//	std::vector<BYTE> propertyBuffer(1024);
//	ULONG propertyBufferSize = static_cast<ULONG>(propertyBuffer.size());
//
//	CONFIGRET cr = CM_Get_Device_Interface_PropertyW(
//		deviceInterface.c_str(),
//		&propertyKey,
//		&propertyType,
//		propertyBuffer.data(),
//		&propertyBufferSize,
//		0);
//
//	if (cr == CR_BUFFER_SMALL)
//	{
//		 如果缓冲区太小，调整缓冲区大小并重试
//		propertyBuffer.resize(propertyBufferSize);
//		cr = CM_Get_Device_Interface_PropertyW(
//			deviceInterface.c_str(),
//			&propertyKey,
//			&propertyType,
//			propertyBuffer.data(),
//			&propertyBufferSize,
//			0);
//	}
//
//	if (cr == CR_SUCCESS)
//	{
//		std::wcout << L"Property retrieved successfully." << std::endl;
//
//		if (propertyType == DEVPROP_TYPE_STRING)
//		{
//			std::wstring propertyValue(reinterpret_cast<wchar_t*>(propertyBuffer.data()));
//			std::wcout << L"Property Value: " << propertyValue.c_str() << std::endl;
//		}
//		else
//		{
//			std::wcout << L"Property is not a string." << std::endl;
//		}
//	}
//	else if (cr == CR_INVALID_DATA)
//	{
//		std::wcout << L"Invalid data or property does not exist." << std::endl;
//	}
//	else
//	{
//		std::wcout << L"Failed to retrieve property. Error code: " << cr << std::endl;
//	}
//}


//int GetHUBInfo(TCHAR *HubName, int nPort) {
//	TCHAR symblName[1024] = { 0 };
//
//	if ((_tcsncicmp(HubName, _T("\\\\?\\"), 4) == 0) || (_tcsncicmp(HubName, _T("\\\\.\\"), 4) == 0))
//	{
//		_stprintf_s(symblName, _T("%ws"), HubName);
//	}
//	else
//	{
//		_stprintf_s(symblName, _T("\\\\?\\%ws"), HubName);
//	}
//	DEVPROPTYPE PropertyType;
//	DWORD requiredSize = 2048;
//
//	std::vector<BYTE> buffer(requiredSize);
//	CONFIGRET ret;
//	if (CR_SUCCESS==(ret = CM_Get_Device_Interface_Property(
//		symblName,
//		&DEVPKEY_Device_Driver,
//		&PropertyType,
//		buffer.data(),
//		&requiredSize,
//		0))) {
//		TCHAR* DeviceLocationpaths = reinterpret_cast<TCHAR*>(buffer.data());
//		std::wcout << L"Hub Location Paths: " << DeviceLocationpaths << std::endl;
//	}
//	else {
//		std::wcout << L"get hub location paths failed: " << ret << std::endl;
//	}
//	return 0;
//}

int GetDeviceLocationPaths(TCHAR *HubName, std::wstring &sLocpath)
{
	TCHAR symblName[1024] = { 0 };
	
	if ((_tcsncicmp(HubName, _T("\\\\?\\"), 4) == 0) || (_tcsncicmp(HubName, _T("\\\\.\\"), 4) == 0))
	{
		_stprintf_s(symblName, _T("%ws"), HubName);
	}
	else
	{
		_stprintf_s(symblName, _T("\\\\?\\%ws"), HubName);
	}
	int nRet = ERROR_SUCCESS;
	HDEVINFO hDevInfo = SetupDiCreateDeviceInfoList(NULL, NULL);
	if (hDevInfo != INVALID_HANDLE_VALUE)
	{
		DWORD sz = 0;
		SP_DEVICE_INTERFACE_DATA devIntData;
		devIntData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		std::wcout << symblName << std::endl;
		if (SetupDiOpenDeviceInterface(hDevInfo, symblName, 0, &devIntData))
		{
			DEVPROPTYPE type;
			BYTE b[2048];
			ZeroMemory(b, sizeof(b));
			sz = 0;
			SP_DEVINFO_DATA devInfoData = { 0 };
			devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
			SetupDiGetDeviceInterfaceDetail(hDevInfo, &devIntData, NULL, 0, &sz, &devInfoData);
			nRet = GetLastError();
			if (nRet == ERROR_INSUFFICIENT_BUFFER)
			{
				if (SetupDiGetDeviceProperty(hDevInfo, &devInfoData, &DEVPKEY_Device_LocationPaths, &type, b, 2048, &sz, 0))
				{
					if (type == DEVPROP_TYPE_STRING_LIST)
					{
						TCHAR *instanceid = (TCHAR *)(b);
						sLocpath = instanceid;
						//std::wcout << sLocpath.c_str() << std::endl;
						logIt(_T("found: %s"), instanceid);
						/*DWORD dwoffset = 0;
						BOOL bFind = FALSE;
						while (instanceid != NULL && _tcslen(instanceid) > 0)
						{
							std::wcout << instanceid << std::endl;

							dwoffset += _tcslen(instanceid) + 1;
							instanceid = (TCHAR *)b + dwoffset;
						}*/
						nRet = ERROR_SUCCESS;
					}
				}
				else
				{
					nRet = GetLastError();
					//std::wcout << L"get hub location paths failed: " << nRet << std::endl;
					logIt(_T("get hub location paths failed: %d"), nRet);
				}
			}
			else
			{
				//nRet = GetLastError();
				//std::wcout << L"get hub SetupDiGetDeviceInterfaceDetail failed: " << nRet << std::endl;
				logIt(_T("get hub SetupDiGetDeviceInterfaceDetail failed: %d"), nRet);
			}
		}
		else
		{
			nRet = GetLastError();
			//std::wcout << L"get SetupDiOpenDeviceInterfaces failed: " << nRet << std::endl;
			logIt(_T("get SetupDiOpenDeviceInterfaces failed: %d"), nRet);
		}

		SetupDiDestroyDeviceInfoList(hDevInfo);
	}
	else
	{
		nRet = GetLastError();
		//std::wcout << L"get SetupDiCreateDeviceInfoList failed: " << nRet << std::endl;
		logIt(_T("get SetupDiCreateDeviceInfoList failed: %d"), nRet);
	}
	
	return nRet;
}

std::string toUTF8String(const std::wstring& wstr)
{
	//s.toUTF8String(result);
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string result(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &result[0], size_needed, NULL, NULL);

	return result;
}

int PrintDeivce(std::wstring hubname, int hubport, std::wstring &devicepath) {
	std::wstring locpath;
	GetDeviceLocationPaths((TCHAR *)hubname.c_str(), locpath);
	if (locpath.size() == 0) {
		logIt(_T("current Felica locationpaths failed"));
		return ERROR_DEVICE_ENUMERATION_ERROR;
	}
	locpath += _T("#USB(") + std::to_wstring(hubport) + _T(")#USBMI(");
	//std::wcout << locpath << std::endl;
	logIt(_T("current Felica locationpaths: %s"), locpath.c_str());

	// Define the interface class GUID for the devices you are interested in
	// Example: GUID_DEVINTERFACE_USB_DEVICE
	//GUID InterfaceClassGuid = { 0xA5DCBF10L, 0x6530, 0x11D2, { 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED } };
	//0c729742-85a4-4105-8fcd-579e5f2d821e
	GUID InterfaceClassGuid = { 0x0c729742L, 0x85a4, 0x4105, { 0x8f, 0xcd, 0x57, 0x9e, 0x5F, 0x2d, 0x82, 0x1E } };

	// Get device information set for the specified interface class
	HDEVINFO DeviceInfoSet = SetupDiGetClassDevs(&InterfaceClassGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (DeviceInfoSet == INVALID_HANDLE_VALUE) {
		logIt(_T("Error getting device information set."));
		return 1;
	}

	// Enumerate the device interfaces
	SP_DEVICE_INTERFACE_DATA DeviceInterfaceData;
	DeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	//extern int IndexSymblinks;

	for (DWORD i = 0; SetupDiEnumDeviceInterfaces(DeviceInfoSet, NULL, &InterfaceClassGuid, i, &DeviceInterfaceData); ++i) {
		// Get the required buffer size
		DWORD requiredSize = 2048;
		//SetupDiGetDeviceInterfaceDetail(DeviceInfoSet, &DeviceInterfaceData, NULL, 0, &requiredSize, NULL);

		// Allocate buffer for SP_DEVICE_INTERFACE_DETAIL_DATA
		std::vector<BYTE> buffer(requiredSize);
		SP_DEVICE_INTERFACE_DETAIL_DATA* DeviceInterfaceDetailData = reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA*>(buffer.data());
		DeviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		// Get the device interface detail
		SP_DEVINFO_DATA devInfoData = { 0 };
		devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		if (SetupDiGetDeviceInterfaceDetail(DeviceInfoSet, &DeviceInterfaceData, DeviceInterfaceDetailData, requiredSize, NULL, &devInfoData)) {
			//std::wcout << L"Device Path: " << DeviceInterfaceDetailData->DevicePath << std::endl;
			logIt(_T("Device Path: %s"), DeviceInterfaceDetailData->DevicePath);
			devicepath = DeviceInterfaceDetailData->DevicePath;
			//std::wstringstream wss;
			//wss << std::setw(2) << std::setfill(L'0') << i;
			//devicepath += _T("\\U*")+wss.str();
			
			DEVPROPTYPE devPropType;
			if (SetupDiGetDeviceProperty(
				DeviceInfoSet,
				&devInfoData,
				&DEVPKEY_Device_LocationPaths,
				&devPropType,
				buffer.data(),
				requiredSize,
				NULL,
				0)) {
				TCHAR* DeviceLocationpaths = reinterpret_cast<TCHAR*>(buffer.data());
				//std::wcout << L"get cur    Location Paths: " << DeviceLocationpaths << std::endl;
				//std::wcout << L"get usbhub Location Paths: " << locpath << std::endl;
				logIt(_T("get cur    Location Paths: %s"), DeviceLocationpaths);
				logIt(_T("get usbhub Location Paths: %s"), locpath.c_str());
				if (StartsWith(DeviceLocationpaths, locpath)) {
					//std::wcout << L"found " << std::endl;
					logIt(_T("found: %s==>%s"), DeviceLocationpaths, locpath.c_str());
					break;
				}
				else {
					devicepath.clear();
				}
			}
			else {
				//std::wcout << L"get location paths failed: " << GetLastError() << std::endl;
				logIt(_T("get location paths failed: %d"), GetLastError());
				devicepath.clear();
			}
			/*if (SetupDiGetDeviceProperty(
				DeviceInfoSet,
				&devInfoData,
				&DEVPKEY_Device_Driver,
				&devPropType,
				buffer.data(),
				requiredSize,
				NULL,
				0)) {
				TCHAR* DeviceDriver = reinterpret_cast<TCHAR*>(buffer.data());
				std::wcout << L"Driver: " << DeviceDriver << std::endl;
			}
			else {
				std::wcout << L"get Driver failed: " << GetLastError() << std::endl;
			}*/
		}
		else {
			//std::cerr << "Error getting device interface detail." << std::endl;
			logIt(_T("Error getting device interface detail"));
		}
	}

	// Clean up
	SetupDiDestroyDeviceInfoList(DeviceInfoSet);
	return 0;
}

std::wstring ToLower(const std::wstring& str) {
	std::wstring result = str;
	std::transform(result.begin(), result.end(), result.begin(), towlower);
	return result;
}

// 忽略大小写的查找函数
size_t CaseInsensitiveFind(const std::wstring& str, const std::wstring& substr) {
	std::wstring lowerStr = ToLower(str);
	std::wstring lowerSubstr = ToLower(substr);
	std::replace(lowerSubstr.begin(), lowerSubstr.end(), L'\\', L'#');
	return lowerStr.find(lowerSubstr);
}

std::wstring GetDeviceInstanceIDFromHubPort(const std::wstring& hubName, int portNumber) {

	logIt(_T("GetDeviceInstanceIDFromHubPort++ %d@%s"), portNumber, hubName.c_str());

	HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&GUID_DEVINTERFACE_USB_DEVICE, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (deviceInfoSet == INVALID_HANDLE_VALUE) {
		logIt(_T("Failed to get device info set"));
		return L"";
	}

	SP_DEVINFO_DATA deviceInfoData;
	deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
	deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	for (DWORD i = 0; SetupDiEnumDeviceInterfaces(deviceInfoSet, nullptr, &GUID_DEVINTERFACE_USB_DEVICE, i, &deviceInterfaceData); ++i) {
		DWORD requiredSize = 0;
		SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, nullptr, 0, &requiredSize, nullptr);

		std::vector<BYTE> buffer(requiredSize);
		PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetailData = reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA>(&buffer[0]);
		deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		if (!SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, deviceInterfaceDetailData, requiredSize, nullptr, &deviceInfoData)) {
			logIt(_T("Failed to get device interface detail"));
			continue;
		}

		WCHAR deviceInstanceID[MAX_DEVICE_ID_LEN];
		if (CM_Get_Device_ID(deviceInfoData.DevInst, deviceInstanceID, MAX_DEVICE_ID_LEN, 0) != CR_SUCCESS) {
			logIt(_T("Failed to get device instance ID"));
			continue;
		}
		// Get the parent device (the hub) of the USB device
		DEVINST parentDevInst;
		if (CM_Get_Parent(&parentDevInst, deviceInfoData.DevInst, 0) != CR_SUCCESS) {
			logIt(_T("Failed to get parent device"));
			continue;
		}

		WCHAR parentDeviceInstanceID[MAX_DEVICE_ID_LEN];
		if (CM_Get_Device_ID(parentDevInst, parentDeviceInstanceID, MAX_DEVICE_ID_LEN, 0) != CR_SUCCESS) {
			logIt(_T("Failed to get parent device instance ID"));
			continue;
		}

		std::wstring parentID(parentDeviceInstanceID);
		logIt(_T("parent Instanceid: %s"), parentDeviceInstanceID);
		if (CaseInsensitiveFind(hubName, parentID) != std::wstring::npos) {
			logIt(_T("parent Instanceidcompare: %s"), parentDeviceInstanceID);

			DWORD port;
			ULONG len = sizeof(DWORD);
			if (CM_Get_DevNode_Registry_Property(deviceInfoData.DevInst, CM_DRP_ADDRESS, nullptr, &port, &len, 0) == CR_SUCCESS) {
				if (port == portNumber) {
					SetupDiDestroyDeviceInfoList(deviceInfoSet);
					return deviceInstanceID;
				}
			}
		}
	}

	SetupDiDestroyDeviceInfoList(deviceInfoSet);
	return L"";
}

int RunExe(std::wstring instanceid){
	// 定义要运行的命令和参数
	std::wstring cmd = L"pnputil.exe /restart-device  \"";
	cmd += instanceid;
	cmd += L"\"";
	// 定义启动信息和进程信息结构体
	STARTUPINFO si;
	PROCESS_INFORMATION pi;


	logIt(_T("RunExe: %s"), cmd.c_str());
	// 初始化启动信息结构体
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	int nRet = ERROR_SUCCESS;
	// 创建新进程
	if (!CreateProcess(
		NULL,            // 应用程序名称
		const_cast<LPWSTR>(cmd.c_str()), // 命令行参数
		NULL,            // 进程安全属性
		NULL,            // 主线程安全属性
		FALSE,           // 继承句柄标志
		0,               // 创建标志
		NULL,            // 环境变量
		NULL,            // 当前目录
		&si,             // 启动信息
		&pi)) {          // 进程信息
		nRet = GetLastError();
		std::cerr << "CreateProcess failed (" << nRet << ").\n";
		return nRet;
	}

	// 等待进程结束
	if (WaitForSingleObject(pi.hProcess, 10 * 1000) != WAIT_OBJECT_0) {
		nRet = GetLastError();
	}

	// 关闭进程和线程句柄
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	std::cout << "pnputil.exe executed successfully." << std::endl;

	return nRet;
}