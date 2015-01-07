#include  <Uefi.h>
#include  <Library/UefiBootServicesTableLib.h>
#include  <Library/DevicePathLib.h>
#include  <Library/ShellCEntryLib.h>
#include  <Library/ShellLib.h>
#include  <Library/UefiLib.h>
#include  <Protocol/LoadedImage.h>

INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
	EFI_STATUS					Status;
	CHAR16						*FileName = L"\\uefi-app-loop.efi";
	EFI_DEVICE_PATH_PROTOCOL	*FilePath = NULL;
	EFI_HANDLE					TargetAppHandle = NULL;
	EFI_HANDLE					*FileSystemHandles = NULL;
	UINTN						NumberFileSystemHandles = 0;
	CHAR16						*FilePathText = NULL;

	// Get simple file system handles
	Status = gBS->LocateHandleBuffer(
			ByProtocol,
			&gEfiSimpleFileSystemProtocolGuid,
			NULL,
			&NumberFileSystemHandles,
			&FileSystemHandles
			);
	if(EFI_ERROR(Status)){
		Print(L"Error: LocateHandleBuffer failed\n");
		return(-1);
	}

	// Get device path of the file
	FilePath = FileDevicePath(FileSystemHandles[0], FileName);
	if(FilePath == NULL){
		Print(L"Error: DeviceFilePath\n");
		return(-1);
	}

	if((FilePathText = ConvertDevicePathToText(FilePath, FALSE, FALSE)) != NULL){
		Print(L"FilePath = %s\n", FilePathText);
	}

	// Load data of the file into memory
	Status = gBS->LoadImage(
			TRUE,
			gImageHandle,
			FilePath,
			NULL,
			0,
			&TargetAppHandle);

	if(EFI_ERROR(Status)){
		Print(L"Error: LoadImage\n");
		return(-1);
	}
	Print(L"TargetAppHandle = 0x%x\n", TargetAppHandle);

	// Execute the image
	Status = gBS->StartImage(TargetAppHandle, 0, 0);
	Print(L"StartImage, Return Value = %d(0x%x)\n", Status, Status);
	
	return(0);
}

