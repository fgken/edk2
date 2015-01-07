#include  <Uefi.h>
#include  <Library/UefiBootServicesTableLib.h>
#include  <Library/DevicePathLib.h>
#include  <Library/MemoryAllocationLib.h>
#include  <Library/ShellCEntryLib.h>
#include  <Library/ShellLib.h>
#include  <Library/UefiLib.h>
#include  <Protocol/LoadedImage.h>
#include  <Protocol/LoadFile.h>

//// ArmPlatformPkg/Library/ArmShellCmdRunAxf/ElfLoader.c
//typedef VOID (*ELF_ENTRYPOINT)(UINTN arg0, UINTN arg1,
//                               UINTN arg2, UINTN arg3);
//
///**
// Load an ELF segment into memory.
//
// This function assumes the ELF file is valid.
// This function is meant to be called for PT_LOAD type segments only.
//**/
//STATIC
//EFI_STATUS
//ElfLoadSegment (
//  IN  CONST VOID  *ElfImage,
//  IN  CONST VOID  *PHdr,
//  IN  LIST_ENTRY  *LoadList
//  )
//{
//  VOID             *FileSegment;
//  VOID             *MemSegment;
//  UINTN             ExtraZeroes;
//  UINTN             ExtraZeroesCount;
//  RUNAXF_LOAD_LIST *LoadNode;
//
//#ifdef MDE_CPU_ARM
//  Elf32_Phdr  *ProgramHdr;
//  ProgramHdr = (Elf32_Phdr *)PHdr;
//#elif defined(MDE_CPU_AARCH64)
//  Elf64_Phdr  *ProgramHdr;
//  ProgramHdr = (Elf64_Phdr *)PHdr;
//#endif
//
//  ASSERT (ElfImage != NULL);
//  ASSERT (ProgramHdr != NULL);
//
//  FileSegment = (VOID *)((UINTN)ElfImage + ProgramHdr->p_offset);
//  MemSegment = (VOID *)ProgramHdr->p_vaddr;
//
//  // If the segment's memory size p_memsz is larger than the file size p_filesz,
//  // the "extra" bytes are defined to hold the value 0 and to follow the
//  // segment's initialised area.
//  // This is typically the case for the .bss segment.
//  // The file size may not be larger than the memory size.
//  if (ProgramHdr->p_filesz > ProgramHdr->p_memsz) {
//    ShellPrintHiiEx (-1, -1, NULL, STRING_TOKEN (STR_RUNAXF_ELFBADFORMAT), gRunAxfHiiHandle);
//    return EFI_INVALID_PARAMETER;
//  }
//
//  // Load the segment in memory.
//  if (ProgramHdr->p_filesz != 0) {
//    DEBUG ((EFI_D_INFO, "Loading segment from 0x%lx to 0x%lx (size = %ld)\n",
//                 FileSegment, MemSegment, ProgramHdr->p_filesz));
//
//    LoadNode = AllocateRuntimeZeroPool (sizeof (RUNAXF_LOAD_LIST));
//    if (LoadNode == NULL) {
//      return EFI_OUT_OF_RESOURCES;
//    }
//    LoadNode->MemOffset  = (UINTN)MemSegment;
//    LoadNode->FileOffset = (UINTN)FileSegment;
//    LoadNode->Length     = (UINTN)ProgramHdr->p_filesz;
//    InsertTailList (LoadList, &LoadNode->Link);
//  }
//
//  ExtraZeroes = ((UINTN)MemSegment + ProgramHdr->p_filesz);
//  ExtraZeroesCount = ProgramHdr->p_memsz - ProgramHdr->p_filesz;
//  DEBUG ((EFI_D_INFO, "Completing segment with %d zero bytes.\n", ExtraZeroesCount));
//  if (ExtraZeroesCount > 0) {
//    // Extra Node to add the Zeroes.
//    LoadNode = AllocateRuntimeZeroPool (sizeof (RUNAXF_LOAD_LIST));
//    if (LoadNode == NULL) {
//      return EFI_OUT_OF_RESOURCES;
//    }
//    LoadNode->MemOffset  = (UINTN)ExtraZeroes;
//    LoadNode->Zeroes     = TRUE;
//    LoadNode->Length     = ExtraZeroesCount;
//    InsertTailList (LoadList, &LoadNode->Link);
//  }
//
//  return EFI_SUCCESS;
//}
//
///**
//  Load a ELF file.
//
//  @param[in] ElfImage       Address of the ELF file in memory.
//
//  @param[out] EntryPoint    Will be filled with the ELF entry point address.
//
//  @param[out] ImageSize     Will be filled with the ELF size in memory. This will
//                            effectively be equal to the sum of the segments sizes.
//
//  This functon assumes the header is valid and supported as checked with
//  ElfCheckFile().
//
//  @retval EFI_SUCCESS on success.
//  @retval EFI_INVALID_PARAMETER if the ELF file is invalid.
//**/
//EFI_STATUS
//ElfLoadFile (
//  IN  CONST VOID   *ElfImage,
//  OUT VOID        **EntryPoint,
//  OUT LIST_ENTRY   *LoadList
//  )
//{
//  EFI_STATUS    Status;
//  UINT8        *ProgramHdr;
//  UINTN         Index;
//  UINTN         ImageSize;
//
//#ifdef MDE_CPU_ARM
//  Elf32_Ehdr   *ElfHdr;
//  Elf32_Phdr   *ProgramHdrPtr;
//
//  ElfHdr = (Elf32_Ehdr*)ElfImage;
//#elif defined(MDE_CPU_AARCH64)
//  Elf64_Ehdr   *ElfHdr;
//  Elf64_Phdr   *ProgramHdrPtr;
//
//  ElfHdr = (Elf64_Ehdr*)ElfImage;
//#endif
//
//  ASSERT (ElfImage   != NULL);
//  ASSERT (EntryPoint != NULL);
//  ASSERT (LoadList   != NULL);
//
//  ProgramHdr = (UINT8*)ElfImage + ElfHdr->e_phoff;
//  DEBUG ((EFI_D_INFO, "ELF program header entry : 0x%lx\n", ProgramHdr));
//
//  ImageSize = 0;
//
//  // Load every loadable ELF segment into memory.
//  for (Index = 0; Index < ElfHdr->e_phnum; ++Index) {
//
//#ifdef MDE_CPU_ARM
//    ProgramHdrPtr = (Elf32_Phdr*)ProgramHdr;
//#elif defined(MDE_CPU_AARCH64)
//    ProgramHdrPtr = (Elf64_Phdr*)ProgramHdr;
//#endif
//
//    // Only consider PT_LOAD type segments, ignore others.
//    if (ProgramHdrPtr->p_type == PT_LOAD) {
//      Status = ElfLoadSegment (ElfImage, (VOID *)ProgramHdrPtr, LoadList);
//      if (EFI_ERROR (Status)) {
//        ShellPrintHiiEx (-1, -1, NULL, STRING_TOKEN (STR_RUNAXF_ELFFAILSEG), gRunAxfHiiHandle);
//        return EFI_INVALID_PARAMETER;
//      }
//      ImageSize += ProgramHdrPtr->p_memsz;
//    }
//    ProgramHdr += ElfHdr->e_phentsize;
//  }
//
//  if (ImageSize == 0) {
//    ShellPrintHiiEx (-1, -1, NULL, STRING_TOKEN (STR_RUNAXF_ELFNOSEG), gRunAxfHiiHandle);
//    return EFI_INVALID_PARAMETER;
//  }
//
//  // Return the entry point specified in the ELF header.
//  *EntryPoint = (void*)ElfHdr->e_entry;
//
//  return EFI_SUCCESS;
//}
//
//
//// Program load list created.
//  // Shutdown UEFI, copy and jump to code.
//  if (!IsListEmpty (&LoadList) && !EFI_ERROR (Status)) {
//    // Exit boot services here. This means we cannot return and cannot assume to
//    // have access to UEFI functions.
//    Status = ShutdownUefiBootServices ();
//    if (EFI_ERROR (Status)) {
//      DEBUG ((EFI_D_ERROR,"Can not shutdown UEFI boot services. Status=0x%X\n",
//              Status));
//    } else {
//      // Process linked list. Copy data to Memory.
//      Node = GetFirstNode (&LoadList);
//      while (!IsNull (&LoadList, Node)) {
//        LoadNode = (RUNAXF_LOAD_LIST *)Node;
//        // Do we have data to copy or do we need to set Zeroes (.bss)?
//        if (LoadNode->Zeroes) {
//          ZeroMem ((VOID*)LoadNode->MemOffset, LoadNode->Length);
//        } else {
//          CopyMem ((VOID *)LoadNode->MemOffset, (VOID *)LoadNode->FileOffset,
//                   LoadNode->Length);
//        }
//        Node = GetNextNode (&LoadList, Node);
//      }
//
//      //
//      // Switch off interrupts, caches, mmu, etc
//      //
//      Status = PreparePlatformHardware ();
//      ASSERT_EFI_ERROR (Status);
//
//      StartElf = (ELF_ENTRYPOINT)Entrypoint;
//      StartElf (0,0,0,0);
//








EFI_STATUS
LoadELF64(CHAR16 *FileName)
{
	EFI_STATUS					Status;
	EFI_DEVICE_PATH_PROTOCOL	*FilePath = NULL;
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

	// Load the file into memory
	{
		EFI_DEVICE_PATH_PROTOCOL *DevicePath;
		EFI_HANDLE DeviceHandle;
		EFI_LOAD_FILE_PROTOCOL *LoadFile;
		UINT8 *ImageBuffer = NULL;
		UINTN ImageBufferSize = 0;

		// Get device handle including filepath
		DevicePath = DuplicateDevicePath(FilePath);
		Status = gBS->LocateDevicePath(&gEfiLoadFileProtocolGuid, &DevicePath, &DeviceHandle);
		if(EFI_ERROR(Status)){
			Print(L"Error: LocateDevicePath failed, Status = %d\n", Status);
			return(-1);
		}

		// Get EFI_LOAD_FILE_PROTOCOL
		Status = gBS->HandleProtocol(DeviceHandle, &gEfiLoadFileProtocolGuid, (VOID**)&LoadFile);
		if(EFI_ERROR(Status)){
			Print(L"Error: HandleProtocol failed, Status = %d", Status);
			return(-1);
		}

		// Retrieve the file size
		ImageBuffer = NULL;
		ImageBufferSize = 0;
		Status = LoadFile->LoadFile(
				LoadFile,
				DevicePath,
				FALSE,
				&ImageBufferSize,
				ImageBuffer);
		if(Status == EFI_BUFFER_TOO_SMALL){
			// Allocate memory of buffer
			ImageBuffer = AllocatePool(ImageBufferSize);
			if(ImageBuffer == NULL){
				Print(L"Error: AllocatePool failed\n");
			}

			// Load the file
			Status = LoadFile->LoadFile(
					LoadFile,
					DevicePath,
					FALSE,
					&ImageBufferSize,
					ImageBuffer);
			if(EFI_ERROR(Status)){
				Print(L"Error: LoadFile failed, Status = %d\n", Status);
				return(-1);
			}
		}
		else{
			Print(L"Error: LoadFile, Status = %d\n", Status);
			return(-1);
		}
	}

	return EFI_SUCCESS;
}

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

	LoadELF64(L"\\Main.debug");

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

	// Load pe/coff image of the file into memory
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

