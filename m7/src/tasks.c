#include "tasks.h"
#include "fsl_debug_console.h"

#define TEST_TASK_STACK_SIZE 256
#define TEST_TASK_PRIORITY   2

static void TestTask(void *pvParameters)
{
    (void)pvParameters;

    while (1)
    {
        PRINTF("FreeRTOS task running!\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

BaseType_t Tasks_Init(void)
{
    BaseType_t status;

    status = xTaskCreate(
        TestTask,
        "Test",
        TEST_TASK_STACK_SIZE,
        NULL,
        TEST_TASK_PRIORITY,
        NULL
    );

    return status;
}