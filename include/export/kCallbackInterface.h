#ifndef _K_CALLBACK_INTERFACE_H_
#define _K_CALLBACK_INTERFACE_H_

#include <workhorse/kSched/kSchedTask.h>

typedef void (*kCallbackActivationFn_t)(kSchedTask_t *task);
typedef void (*kCallbackResponseFn_t)(kSchedTask_t *task);
typedef void (*kCallbackCpuHandoffFn_t)(void);

typedef struct kCallbackOps
{
    kCallbackActivationFn_t kCallbackActivationFn;
    kCallbackResponseFn_t kCallbackResponseFn;
    kCallbackCpuHandoffFn_t kCallbackCpuHandoffFn;
} kCallbackOps_t;

int kCallbackOpsInit(kCallbackOps_t *ops);

void kCallbackActivation(kSchedTask_t *task);
void kCallbackResponse(kSchedTask_t *task);
void kCallbackCpuHandoff(void);

#endif