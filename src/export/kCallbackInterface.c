#include <export/kCallbackInterface.h>
#include <errno.h>

static 
kCallbackOps_t *gOps = NULL;

static 
bool gOpsInitialized = false;

int kCallbackOpsInit(kCallbackOps_t *ops)
{
    if (gOpsInitialized)
        return -EINVAL;

    gOps = ops;
    gOpsInitialized = true;
    return 0;
}

void kCallbackActivation(kSchedTask_t *task)
{
    if (gOpsInitialized)
        gOps->kCallbackActivationFn(task);
}

void kCallbackResponse(kSchedTask_t *task)
{
    if (gOpsInitialized)
        gOps->kCallbackResponseFn(task);
}

void kCallbackCpuHandoff(void)
{
    if (gOpsInitialized)
        gOps->kCallbackCpuHandoffFn();
}