#include "mbed.h"
#include <cstdint>
#include "../SoftwareSerial/SoftwareSerial.h"
#include "../pin/pin.h"

//#include "TMC2130_bitfields.h"
//#include "TMC2160_bitfields.h"
//#include "TMC5130_bitfields.h"
//#include "TMC5160_bitfields.h"
#include "TMC2208_bitfields.h"
#include "TMC2209_bitfields.h"
//#include "TMC2660_bitfields.h"


#define INIT_REGISTER(REG) REG##_t REG##_register = REG##_t
#define INIT2208_REGISTER(REG) TMC2208_n::REG##_t REG##_register = TMC2208_n::REG##_t
#define SET_ALIAS(TYPE, DRIVER, NEW, ARG, OLD) TYPE (DRIVER::*NEW)(ARG) = &DRIVER::OLD

#define TMCSTEPPER_VERSION 0x000701 // v0.7.1

class TMCStepper {
    public:
        uint16_t cs2rms(uint8_t CS);
        void rms_current(uint16_t mA);
        void rms_current(uint16_t mA, float mult);
        uint16_t rms_current();
        void hold_multiplier(float val) { holdMultiplier = val; }
        float hold_multiplier() { return holdMultiplier; }
        uint8_t test_connection();

        // Helper functions
        void microsteps(uint16_t ms);
        uint16_t microsteps();
        void blank_time(uint8_t value);
        uint8_t blank_time();
        void hysteresis_end(int8_t value);
        int8_t hysteresis_end();
        void hysteresis_start(uint8_t value);
        uint8_t hysteresis_start();

        // R+WC: GSTAT
        void    GSTAT(                          uint8_t input);
        uint8_t GSTAT();
        bool    reset();
        bool    drv_err();
        bool    uv_cp();

        // W: IHOLD_IRUN
        void IHOLD_IRUN(                    uint32_t input);
        uint32_t IHOLD_IRUN();
        void    ihold(                          uint8_t B);
        void    irun(                               uint8_t B);
        void    iholddelay(                 uint8_t B);
        uint8_t ihold();
        uint8_t irun();
        uint8_t iholddelay();

        // W: TPOWERDOWN
        uint8_t TPOWERDOWN();
        void TPOWERDOWN(                    uint8_t input);

        // R: TSTEP
        uint32_t TSTEP();

        // W: TPWMTHRS
        uint32_t TPWMTHRS();
        void TPWMTHRS(                      uint32_t input);

        // R: MSCNT
        uint16_t MSCNT();

        // R: MSCURACT
        uint32_t MSCURACT();
        int16_t cur_a();
        int16_t cur_b();

		Timer tmcTimer;

    protected:
        TMCStepper(float RS) : Rsense(RS) {};
        INIT_REGISTER(IHOLD_IRUN){{.sr=0}}; // 32b
        INIT_REGISTER(TPOWERDOWN){.sr=0};       // 8b
        INIT_REGISTER(TPWMTHRS){.sr=0};         // 32b

        static constexpr uint8_t TMC_READ = 0x00,
                                 TMC_WRITE = 0x80;

        struct TSTEP_t { constexpr static uint8_t address = 0x12; };
        struct MSCNT_t { constexpr static uint8_t address = 0x6A; };

        virtual void write(uint8_t, uint32_t) = 0;
        virtual uint32_t read(uint8_t) = 0;
        virtual void vsense(bool) = 0;
        virtual bool vsense(void) = 0;
        virtual uint32_t DRV_STATUS() = 0;
        virtual void hend(uint8_t) = 0;
        virtual uint8_t hend() = 0;
        virtual void hstrt(uint8_t) = 0;
        virtual uint8_t hstrt() = 0;
        virtual void mres(uint8_t) = 0;
        virtual uint8_t mres() = 0;
        virtual void tbl(uint8_t) = 0;
        virtual uint8_t tbl() = 0;

        const float Rsense;
        float holdMultiplier = 0.5;
};


class TMC2208Stepper : public TMCStepper {
    public:

        TMC2208Stepper(std::string SWRXpin, std::string SWTXpin, float RS) :
                TMC2208Stepper(SWRXpin, SWTXpin, RS, TMC2208_SLAVE_ADDR)
                {}

        SoftwareSerial * SWSerial = nullptr;

        void defaults();
        void push();
        void begin();
        void beginSerial(uint32_t baudrate) __attribute__((weak));

        bool isEnabled();

        // RW: GCONF
        void GCONF(uint32_t input);
        void I_scale_analog(bool B);
        void internal_Rsense(bool B);
        void en_spreadCycle(bool B);
        void shaft(bool B);
        void index_otpw(bool B);
        void index_step(bool B);
        void pdn_disable(bool B);
        void mstep_reg_select(bool B);
        void multistep_filt(bool B);
        uint32_t GCONF();
        bool I_scale_analog();
        bool internal_Rsense();
        bool en_spreadCycle();
        bool shaft();
        bool index_otpw();
        bool index_step();
        bool pdn_disable();
        bool mstep_reg_select();
        bool multistep_filt();

        // R: IFCNT
        uint8_t IFCNT();

        // W: SLAVECONF
        void SLAVECONF(uint16_t input);
        uint16_t SLAVECONF();
        void senddelay(uint8_t B);
        uint8_t senddelay();

        // W: OTP_PROG
        void OTP_PROG(uint16_t input);

        // R: OTP_READ
        uint32_t OTP_READ();

        // R: IOIN
        uint32_t IOIN();
        bool enn();
        bool ms1();
        bool ms2();
        bool diag();
        bool pdn_uart();
        bool step();
        bool sel_a();
        bool dir();
        uint8_t version();

        // RW: FACTORY_CONF
        void FACTORY_CONF(uint16_t input);
        uint16_t FACTORY_CONF();
        void fclktrim(uint8_t B);
        void ottrim(uint8_t B);
        uint8_t fclktrim();
        uint8_t ottrim();

        // W: VACTUAL
        void VACTUAL(uint32_t input);
        uint32_t VACTUAL();

        // RW: CHOPCONF
        void CHOPCONF(uint32_t input);
        void toff(uint8_t B);
        void hstrt(uint8_t B);
        void hend(uint8_t B);
        void tbl(uint8_t B);
        void vsense(bool B);
        void mres(uint8_t B);
        void intpol(bool B);
        void dedge(bool B);
        void diss2g(bool B);
        void diss2vs(bool B);
        uint32_t CHOPCONF();
        uint8_t toff();
        uint8_t hstrt();
        uint8_t hend();
        uint8_t tbl();
        bool vsense();
        uint8_t mres();
        bool intpol();
        bool dedge();
        bool diss2g();
        bool diss2vs();

        // R: DRV_STATUS
        uint32_t DRV_STATUS();
        bool otpw();
        bool ot();
        bool s2ga();
        bool s2gb();
        bool s2vsa();
        bool s2vsb();
        bool ola();
        bool olb();
        bool t120();
        bool t143();
        bool t150();
        bool t157();
        uint16_t cs_actual();
        bool stealth();
        bool stst();

        // RW: PWMCONF
        void PWMCONF(uint32_t input);
        void pwm_ofs(uint8_t B);
        void pwm_grad(uint8_t B);
        void pwm_freq(uint8_t B);
        void pwm_autoscale(bool B);
        void pwm_autograd(bool B);
        void freewheel(uint8_t B);
        void pwm_reg(uint8_t B);
        void pwm_lim(uint8_t B);
        uint32_t PWMCONF();
        uint8_t pwm_ofs();
        uint8_t pwm_grad();
        uint8_t pwm_freq();
        bool pwm_autoscale();
        bool pwm_autograd();
        uint8_t freewheel();
        uint8_t pwm_reg();
        uint8_t pwm_lim();

        // R: PWM_SCALE
        uint32_t PWM_SCALE();
        uint8_t pwm_scale_sum();
        int16_t pwm_scale_auto();

        // R: PWM_AUTO (0x72)
        uint32_t PWM_AUTO();
        uint8_t pwm_ofs_auto();
        uint8_t pwm_grad_auto();

        uint16_t bytesWritten = 0;
        float Rsense = 0.11;
        bool CRCerror = false;
    protected:
        INIT2208_REGISTER(GCONF)            {{.sr=0}};
        INIT_REGISTER(SLAVECONF)            {{.sr=0}};
        INIT_REGISTER(FACTORY_CONF)     {{.sr=0}};
        INIT2208_REGISTER(VACTUAL)      {.sr=0};
        INIT2208_REGISTER(CHOPCONF)     {{.sr=0}};
        INIT2208_REGISTER(PWMCONF)      {{.sr=0}};

        struct IFCNT_t      { constexpr static uint8_t address = 0x02; };
        struct OTP_PROG_t   { constexpr static uint8_t address = 0x04; };
        struct OTP_READ_t   { constexpr static uint8_t address = 0x05; };

        //SoftwareSerial * SWSerial = nullptr;

        TMC2208Stepper(std::string SWRXpin, std::string SWTXpin, float RS, uint8_t addr);

        std::string SWTXpin;
        std::string SWRXpin;

        DigitalOut* debug1;
        DigitalOut* debug2;

        int available();
        void preWriteCommunication();
        void preReadCommunication();
        int16_t serial_read();
        uint8_t serial_write(const uint8_t data);
        void postWriteCommunication();
        void postReadCommunication();
        void write(uint8_t, uint32_t);
        uint32_t read(uint8_t);
        const uint8_t slave_address;
        uint8_t calcCRC(uint8_t datagram[], uint8_t len);
        static constexpr uint8_t  TMC2208_SYNC = 0x05,
                                  TMC2208_SLAVE_ADDR = 0x00;
        static constexpr uint8_t replyDelay = 2;  //ms
        static constexpr uint8_t abort_window = 5;
        static constexpr uint8_t max_retries = 2;

        uint64_t _sendDatagram(uint8_t [], const uint8_t, uint16_t);
};

class TMC2209Stepper : public TMC2208Stepper {
    public:

        TMC2209Stepper(std::string SWRXpin, std::string SWTXpin, float RS, uint8_t addr) :
                TMC2208Stepper(SWRXpin, SWTXpin, RS, addr) {}

        void push();

        // R: IOIN
        uint32_t IOIN();
        bool enn();
        bool ms1();
        bool ms2();
        bool diag();
        bool pdn_uart();
        bool step();
        bool spread_en();
        bool dir();
        uint8_t version();

        // W: TCOOLTHRS
        uint32_t TCOOLTHRS();
        void TCOOLTHRS(uint32_t input);

        // W: SGTHRS
        void SGTHRS(uint8_t B);
        uint8_t SGTHRS();

        // R: SG_RESULT
        uint16_t SG_RESULT();

        // W: COOLCONF
        void COOLCONF(uint16_t B);
        uint16_t COOLCONF();
        void semin(uint8_t B);
        void seup(uint8_t B);
        void semax(uint8_t B);
        void sedn(uint8_t B);
        void seimin(bool B);
        uint8_t semin();
        uint8_t seup();
        uint8_t semax();
        uint8_t sedn();
        bool seimin();

    protected:
        INIT_REGISTER(TCOOLTHRS){.sr=0};
        TMC2209_n::SGTHRS_t SGTHRS_register{.sr=0};
        TMC2209_n::COOLCONF_t COOLCONF_register{{.sr=0}};
};
