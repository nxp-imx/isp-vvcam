#ifndef _VVFOCUS_H_
#define _VVFOCUS_H_

enum {
    VVFOCUSIOC_GET_RANGE = 0x100,
    VVFOCUSIOC_GET_POS,
    VVFOCUSIOC_SET_POS,
    VVFOCUSIOC_SET_REG,
    VVFOCUSIOC_GET_REG,
    VVFOCUSIOC_MAX,

};

typedef enum vvfocus_mode_e {
    VVFOCUS_MODE_ABSOLUTE = 0,
    VVFOCUS_MODE_RELATIVE,
} vvfocus_mode_t;

struct vvfocus_reg_s {
    uint32_t addr;
    uint32_t value;
};

struct vvfocus_range_s {
    int32_t min_pos;
    int32_t max_pos;
    uint32_t step;
};

struct vvfocus_pos_s {
    vvfocus_mode_t mode;
    int32_t pos;
};

#endif
