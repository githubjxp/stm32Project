/**
 * @file
 * @author jiangxiaoping
 * @date 2025-04-17
 * @version v1.0.0
 */

typedef struct {
    void (*init)(void);
    void (*read)(void);
    void (*write)(void);
    void (*deinit)(void);
} DriverOps_t;

typedef struct {
    const char *name;         // Name of the driver
    uint32_t id;              // Unique ID for the driver
    DriverOps_t *ops;         // Pointer to driver operations
} Driver_t;