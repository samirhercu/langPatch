#include <pspkernel.h>
#include <systemctrl.h>

PSP_MODULE_INFO("langPatcher_TxtMod", 0x1000, 1, 0);

u32 k1;
static int languagePatch = -1;
static int (*sceUtilityGetSystemParamInt)( int id, int *value ) = NULL;

int sceUtilityGetSystemParamInt_patched( int id, int *value )
{
    int ret;
    
    k1 = pspSdkGetK1();
    
    ret = sceUtilityGetSystemParamInt(id, value);
    if (ret != 0) goto noPatch_exit;
    if (id == 8)
    {
        *value = languagePatch;
    }

noPatch_exit:
    pspSdkSetK1(k1);
    
    return ret;
}

int module_start( SceSize args, void *argp )
{
    SceUID file = sceIoOpen("ms0:/seplugins/language.txt", PSP_O_RDONLY, 0777);
    
    if (file >= 0)
    {
        char buffer[8] = {0};
        int bytesRead = sceIoRead(file, buffer, 4); 
        sceIoClose(file);

        if (bytesRead > 0)
        {
            int parsedVal = 0;
            int i = 0;
            int foundDigit = 0;

            while(i < bytesRead)
            {
                if(buffer[i] >= '0' && buffer[i] <= '9')
                {
                    parsedVal = (parsedVal * 10) + (buffer[i] - '0');
                    foundDigit = 1;
                }
                else if (foundDigit) 
                {
                    break; 
                }
                i++;
            }

            if (foundDigit)
            {
                languagePatch = parsedVal;
            }
        }
    }
    else
    {
        return 0;
    }

    if ((languagePatch < 0) || (languagePatch > 11))
    {
        return 0;
    }
    
    u32 functionAddress = sctrlHENFindFunction("sceUtility_Driver", "sceUtility", 0xA5DA2406);
    sceUtilityGetSystemParamInt = (void *)functionAddress;
    
    sctrlHENPatchSyscall((void *)functionAddress, sceUtilityGetSystemParamInt_patched);
    sceKernelDcacheWritebackAll();
    sceKernelIcacheClearAll();

    return 0;
}

int module_stop( SceSize args, void *argp )
{
    return 0;
}