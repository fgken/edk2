#include  <Uefi.h>
#include  <Library/UefiBootServicesTableLib.h>
#include  <Library/DevicePathLib.h>
#include  <Library/ShellCEntryLib.h>
#include  <Library/ShellLib.h>
#include  <Library/UefiLib.h>
#include  <Protocol/LoadedImage.h>

// ArmPlatformPkg/Library/ArmShellCmdRunAxf/ElfLoader.c
/**
 Load an ELF segment into memory.

 This function assumes the ELF file is valid.
 This function is meant to be called for PT_LOAD type segments only.
**/
STATIC
EFI_STATUS
ElfLoadSegment (
  IN  CONST VOID  *ElfImage,
  IN  CONST VOID  *PHdr,
  IN  LIST_ENTRY  *LoadList
  )
{
  VOID             *FileSegment;
  VOID             *MemSegment;
  UINTN             ExtraZeroes;
  UINTN             ExtraZeroesCount;
  RUNAXF_LOAD_LIST *LoadNode;

#ifdef MDE_CPU_ARM
  Elf32_Phdr  *ProgramHdr;
  ProgramHdr = (Elf32_Phdr *)PHdr;
#elif defined(MDE_CPU_AARCH64)
  Elf64_Phdr  *ProgramHdr;
  ProgramHdr = (Elf64_Phdr *)PHdr;
#endif

  ASSERT (ElfImage != NULL);
  ASSERT (ProgramHdr != NULL);

  FileSegment = (VOID *)((UINTN)ElfImage + ProgramHdr->p_offset);
  MemSegment = (VOID *)ProgramHdr->p_vaddr;

  // If the segment's memory size p_memsz is larger than the file size p_filesz,
  // the "extra" bytes are defined to hold the value 0 and to follow the
  // segment's initialised area.
  // This is typically the case for the .bss segment.
  // The file size may not be larger than the memory size.
  if (ProgramHdr->p_filesz > ProgramHdr->p_memsz) {
    ShellPrintHiiEx (-1, -1, NULL, STRING_TOKEN (STR_RUNAXF_ELFBADFORMAT), gRunAxfHiiHandle);
    return EFI_INVALID_PARAMETER;
  }

  // Load the segment in memory.
  if (ProgramHdr->p_filesz != 0) {
    DEBUG ((EFI_D_INFO, "Loading segment from 0x%lx to 0x%lx (size = %ld)\n",
                 FileSegment, MemSegment, ProgramHdr->p_filesz));

    LoadNode = AllocateRuntimeZeroPool (sizeof (RUNAXF_LOAD_LIST));
    if (LoadNode == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    LoadNode->MemOffset  = (UINTN)MemSegment;
    LoadNode->FileOffset = (UINTN)FileSegment;
    LoadNode->Length     = (UINTN)ProgramHdr->p_filesz;
    InsertTailList (LoadList, &LoadNode->Link);
  }

  ExtraZeroes = ((UINTN)MemSegment + ProgramHdr->p_filesz);
  ExtraZeroesCount = ProgramHdr->p_memsz - ProgramHdr->p_filesz;
  DEBUG ((EFI_D_INFO, "Completing segment with %d zero bytes.\n", ExtraZeroesCount));
  if (ExtraZeroesCount > 0) {
    // Extra Node to add the Zeroes.
    LoadNode = AllocateRuntimeZeroPool (sizeof (RUNAXF_LOAD_LIST));
    if (LoadNode == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    LoadNode->MemOffset  = (UINTN)ExtraZeroes;
    LoadNode->Zeroes     = TRUE;
    LoadNode->Length     = ExtraZeroesCount;
    InsertTailList (LoadList, &LoadNode->Link);
  }

  return EFI_SUCCESS;
}

/**
  Load a ELF file.

  @param[in] ElfImage       Address of the ELF file in memory.

  @param[out] EntryPoint    Will be filled with the ELF entry point address.

  @param[out] ImageSize     Will be filled with the ELF size in memory. This will
                            effectively be equal to the sum of the segments sizes.

  This functon assumes the header is valid and supported as checked with
  ElfCheckFile().

  @retval EFI_SUCCESS on success.
  @retval EFI_INVALID_PARAMETER if the ELF file is invalid.
**/
EFI_STATUS
ElfLoadFile (
  IN  CONST VOID   *ElfImage,
  OUT VOID        **EntryPoint,
  OUT LIST_ENTRY   *LoadList
  )
{
  EFI_STATUS    Status;
  UINT8        *ProgramHdr;
  UINTN         Index;
  UINTN         ImageSize;

#ifdef MDE_CPU_ARM
  Elf32_Ehdr   *ElfHdr;
  Elf32_Phdr   *ProgramHdrPtr;

  ElfHdr = (Elf32_Ehdr*)ElfImage;
#elif defined(MDE_CPU_AARCH64)
  Elf64_Ehdr   *ElfHdr;
  Elf64_Phdr   *ProgramHdrPtr;

  ElfHdr = (Elf64_Ehdr*)ElfImage;
#endif

  ASSERT (ElfImage   != NULL);
  ASSERT (EntryPoint != NULL);
  ASSERT (LoadList   != NULL);

  ProgramHdr = (UINT8*)ElfImage + ElfHdr->e_phoff;
  DEBUG ((EFI_D_INFO, "ELF program header entry : 0x%lx\n", ProgramHdr));

  ImageSize = 0;

  // Load every loadable ELF segment into memory.
  for (Index = 0; Index < ElfHdr->e_phnum; ++Index) {

#ifdef MDE_CPU_ARM
    ProgramHdrPtr = (Elf32_Phdr*)ProgramHdr;
#elif defined(MDE_CPU_AARCH64)
    ProgramHdrPtr = (Elf64_Phdr*)ProgramHdr;
#endif

    // Only consider PT_LOAD type segments, ignore others.
    if (ProgramHdrPtr->p_type == PT_LOAD) {
      Status = ElfLoadSegment (ElfImage, (VOID *)ProgramHdrPtr, LoadList);
      if (EFI_ERROR (Status)) {
        ShellPrintHiiEx (-1, -1, NULL, STRING_TOKEN (STR_RUNAXF_ELFFAILSEG), gRunAxfHiiHandle);
        return EFI_INVALID_PARAMETER;
      }
      ImageSize += ProgramHdrPtr->p_memsz;
    }
    ProgramHdr += ElfHdr->e_phentsize;
  }

  if (ImageSize == 0) {
    ShellPrintHiiEx (-1, -1, NULL, STRING_TOKEN (STR_RUNAXF_ELFNOSEG), gRunAxfHiiHandle);
    return EFI_INVALID_PARAMETER;
  }

  // Return the entry point specified in the ELF header.
  *EntryPoint = (void*)ElfHdr->e_entry;

  return EFI_SUCCESS;
}

static grub_err_t
grub_load_elf64 (grub_file_t file, void *buffer, const char *filename)
{
  Elf64_Ehdr *ehdr = (Elf64_Ehdr *) buffer;
  Elf64_Phdr *phdr;
  int i;
  grub_uint64_t low_addr;
  grub_uint64_t high_addr;
  grub_uint64_t align;
  grub_uint64_t reloc_offset;
  const char *relocate;

  if (ehdr->e_ident[EI_MAG0] != ELFMAG0
      || ehdr->e_ident[EI_MAG1] != ELFMAG1
      || ehdr->e_ident[EI_MAG2] != ELFMAG2
      || ehdr->e_ident[EI_MAG3] != ELFMAG3
      || ehdr->e_ident[EI_DATA] != ELFDATA2LSB)
    return grub_error(GRUB_ERR_UNKNOWN_OS,
		      N_("invalid arch-independent ELF magic"));

  if (ehdr->e_ident[EI_CLASS] != ELFCLASS64
      || ehdr->e_version != EV_CURRENT
      || ehdr->e_machine != EM_IA_64)
    return grub_error (GRUB_ERR_UNKNOWN_OS,
		       N_("invalid arch-dependent ELF magic"));

  if (ehdr->e_type != ET_EXEC)
    return grub_error (GRUB_ERR_UNKNOWN_OS,
		       N_("this ELF file is not of the right type"));

  /* FIXME: Should we support program headers at strange locations?  */
  if (ehdr->e_phoff + ehdr->e_phnum * ehdr->e_phentsize > GRUB_ELF_SEARCH)
    return grub_error (GRUB_ERR_BAD_OS, "program header at a too high offset");

  entry = ehdr->e_entry;

  /* Compute low, high and align addresses.  */
  low_addr = ~0UL;
  high_addr = 0;
  align = 0;
  for (i = 0; i < ehdr->e_phnum; i++)
    {
      phdr = (Elf64_Phdr *) ((char *) buffer + ehdr->e_phoff
			     + i * ehdr->e_phentsize);
      if (phdr->p_type == PT_LOAD)
	{
	  if (phdr->p_paddr < low_addr)
	    low_addr = phdr->p_paddr;
	  if (phdr->p_paddr + phdr->p_memsz > high_addr)
	    high_addr = phdr->p_paddr + phdr->p_memsz;
	  if (phdr->p_align > align)
	    align = phdr->p_align;
	}
    }

  if (align < ALIGN_MIN)
    align = ALIGN_MIN;

  if (high_addr == 0)
    return grub_error (GRUB_ERR_BAD_OS, "no program entries");

  kernel_pages = page_align (high_addr - low_addr) >> 12;

  /* Undocumented on purpose.  */
  relocate = grub_env_get ("linux_relocate");
  if (!relocate || grub_strcmp (relocate, "force") != 0)
    {
      kernel_mem = grub_efi_allocate_pages (low_addr, kernel_pages);
      reloc_offset = 0;
    }
  /* Try to relocate.  */
  if (! kernel_mem && (!relocate || grub_strcmp (relocate, "off") != 0))
    {
      kernel_mem = allocate_pages (align, kernel_pages, low_addr);
      if (kernel_mem)
	{
	  reloc_offset = (grub_uint64_t)kernel_mem - low_addr;
	  grub_dprintf ("linux", "  Relocated at %p (offset=%016lx)\n",
			kernel_mem, reloc_offset);
	  entry += reloc_offset;
	}
    }
  if (! kernel_mem)
    return grub_error (GRUB_ERR_OUT_OF_MEMORY,
		       "cannot allocate memory for OS");

  /* Load every loadable segment in memory.  */
  for (i = 0; i < ehdr->e_phnum; i++)
    {
      phdr = (Elf64_Phdr *) ((char *) buffer + ehdr->e_phoff
			     + i * ehdr->e_phentsize);
      if (phdr->p_type == PT_LOAD)
        {
	  grub_dprintf ("linux", "  [paddr=%lx load=%lx memsz=%08lx "
			"off=%lx flags=%x]\n",
			phdr->p_paddr, phdr->p_paddr + reloc_offset,
			phdr->p_memsz, phdr->p_offset, phdr->p_flags);
	  
	  if (grub_file_seek (file, phdr->p_offset) == (grub_off_t)-1)
	    return grub_errno;

	  if (grub_file_read (file, (void *) (phdr->p_paddr + reloc_offset),
			      phdr->p_filesz)
              != (grub_ssize_t) phdr->p_filesz)
	    {
	      if (!grub_errno)
		grub_error (GRUB_ERR_BAD_OS, N_("premature end of file %s"),
			    filename);
	      return grub_errno;
	    }
	  
          if (phdr->p_filesz < phdr->p_memsz)
	    grub_memset
	      ((char *)(phdr->p_paddr + reloc_offset + phdr->p_filesz),
	       0, phdr->p_memsz - phdr->p_filesz);

	  /* Sync caches if necessary.  */
	  if (phdr->p_flags & PF_X)
	    grub_arch_sync_caches
	      ((void *)(phdr->p_paddr + reloc_offset), phdr->p_memsz);
        }
    }
  loaded = 1;
  return 0;
}


EFI_STATUS
LoadELF64(CHAR16 *FileName)
{
	EFI_STATUS					Status;
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

