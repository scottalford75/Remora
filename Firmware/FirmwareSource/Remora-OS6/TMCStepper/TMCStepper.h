#include "mbed.h"
#include <cstdint>
#include "../SoftwareSerial/SoftwareSerial.h"
#include "../pin/pin.h"

//#include "TMC2130_bitfields.h"
#include "TMC2160_bitfields.h"
#include "TMC5130_bitfields.h"
#include "TMC5160_bitfields.h"
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

class TMC5130Stepper : public TMC2160Stepper {
	public:
		TMC5130Stepper(uint16_t pinCS, float RS = default_RS, int8_t link_index = -1);
		TMC5130Stepper(uint16_t pinCS, uint16_t pinMOSI, uint16_t pinMISO, uint16_t pinSCK, int8_t link_index = -1);
		TMC5130Stepper(uint16_t pinCS, float RS, uint16_t pinMOSI, uint16_t pinMISO, uint16_t pinSCK, int8_t link_index = -1);

		void begin();
		void defaults();
		void push();

		void rms_current(uint16_t mA) { TMC2130Stepper::rms_current(mA); }
		void rms_current(uint16_t mA, float mult) { TMC2130Stepper::rms_current(mA, mult); }
		uint16_t rms_current() { return TMC2130Stepper::rms_current(); }

		// R: IFCNT
		uint8_t IFCNT();
		// W: SLAVECONF
		uint16_t SLAVECONF();
		void SLAVECONF(uint16_t input);
		// R: IOIN
		uint32_t 	IOIN();
		bool 			refl_step();
		bool 			refr_dir();
		bool 			encb_dcen_cfg4();
		bool 			enca_dcin_cfg5();
		bool 			drv_enn_cfg6();
		bool 			enc_n_dco();
		bool 			sd_mode();
		bool 			swcomp_in();
		uint8_t 	version();

		// RW: GCONF
		void diag1_poscomp_pushpull(bool B) { diag1_pushpull(B); }
		bool diag1_poscomp_pushpull() { return diag1_pushpull(); }

		// RW: SW_MODE
		uint32_t SW_MODE();
		void SW_MODE(uint32_t input);

		void stop_l_enable(bool B);
		void stop_r_enable(bool B);
		void pol_stop_l(bool B);
		void pol_stop_r(bool B);
		void swap_lr(bool B);
		void latch_l_active(bool B);
		void latch_l_inactive(bool B);
		void latch_r_active(bool B);
		void latch_r_inactive(bool B);
		void en_latch_encoder(bool B);
		void sg_stop(bool B);
		void en_softstop(bool B);

		bool stop_r_enable();
		bool pol_stop_l();
		bool pol_stop_r();
		bool swap_lr();
		bool latch_l_active();
		bool latch_l_inactive();
		bool latch_r_active();
		bool latch_r_inactive();
		bool en_latch_encoder();
		bool sg_stop();
		bool en_softstop();

		// R+C: RAMP_STAT
		uint32_t RAMP_STAT();
		bool status_stop_l();
		bool status_stop_r();
		bool status_latch_l();
		bool status_latch_r();
		bool event_stop_l();
		bool event_stop_r();
		bool event_stop_sg();
		bool event_pos_reached();
		bool velocity_reached();
		bool position_reached();
		bool vzero();
		bool t_zerowait_active();
		bool second_move();
		bool status_sg();

		// RW: ENCMODE
		uint32_t ENCMODE();
		void ENCMODE(uint32_t input);
		void pol_a(bool B);
		void pol_b(bool B);
		void pol_n(bool B);
		void ignore_ab(bool B);
		void clr_cont(bool B);
		void clr_once(bool B);
		void pos_edge(bool B);
		void neg_edge(bool B);
		void clr_enc_x(bool B);
		void latch_x_act(bool B);
		void enc_sel_decimal(bool B);
		bool pol_a();
		bool pol_b();
		bool pol_n();
		bool ignore_ab();
		bool clr_cont();
		bool clr_once();
		bool pos_edge();
		bool neg_edge();
		bool clr_enc_x();
		bool latch_x_act();
		bool enc_sel_decimal();

		// W: OUTPUT
		bool TMC_OUTPUT();
		void TMC_OUTPUT(bool input);
		// W: X_COMPARE
		uint32_t X_COMPARE();
		void X_COMPARE(uint32_t input);
		// RW: RAMPMODE
		uint8_t RAMPMODE();
		void RAMPMODE(uint8_t input);
		// RW: XACTUAL
		int32_t XACTUAL();
		void XACTUAL(int32_t input);
		// R: VACTUAL
		int32_t VACTUAL();
		// W: VSTART
		uint32_t VSTART();
		void VSTART(uint32_t input);
		// W: A1
		uint16_t a1();
		void a1(uint16_t input);
		// W: V1
		uint32_t v1();
		void v1(uint32_t input);
		// W: AMAX
		uint16_t AMAX();
		void AMAX(uint16_t input);
		// W: VMAX
		uint32_t VMAX();
		void VMAX(uint32_t input);
		// W: DMAX
		uint16_t DMAX();
		void DMAX(uint16_t input);
		// W: D1
		uint16_t d1();
		void d1(uint16_t input);
		// W: VSTOP
		uint32_t VSTOP();
		void VSTOP(uint32_t input);
		// W: TZEROWAIT
		uint16_t TZEROWAIT();
		void TZEROWAIT(uint16_t input);
		// RW: XTARGET
		int32_t XTARGET();
		void XTARGET(int32_t input);
		// R: XLATCH
		uint32_t XLATCH();
		// RW: X_ENC
		int32_t X_ENC();
		void X_ENC(int32_t input);
		// W: ENC_CONST
		uint32_t ENC_CONST();
		void ENC_CONST(uint32_t input);
		// R: ENC_STATUS
		bool ENC_STATUS();
		// R: ENC_LATCH
		uint32_t ENC_LATCH();

		using TMC2130Stepper::ENCM_CTRL;

		using TMC2130Stepper::PWMCONF;
		using TMC2130Stepper::pwm_ampl;
		using TMC2130Stepper::pwm_grad;
		using TMC2130Stepper::pwm_freq;
		using TMC2130Stepper::pwm_autoscale;
		using TMC2130Stepper::pwm_symmetric;
		using TMC2130Stepper::freewheel;

		using TMC2130Stepper::PWM_SCALE;

	protected:
		INIT_REGISTER(SLAVECONF){{.sr=0}};
		INIT_REGISTER(OUTPUT){.sr=0};
		INIT_REGISTER(X_COMPARE){.sr=0};
		INIT_REGISTER(RAMPMODE){.sr=0};
		INIT_REGISTER(XACTUAL){.sr=0};
		INIT_REGISTER(VSTART){.sr=0};
		INIT_REGISTER(A1){.sr=0};
		INIT_REGISTER(V1){.sr=0};
		INIT_REGISTER(AMAX){.sr=0};
		INIT_REGISTER(VMAX){.sr=0};
		INIT_REGISTER(DMAX){.sr=0};
		INIT_REGISTER(D1){.sr=0};
		INIT_REGISTER(VSTOP){.sr=0};
		INIT_REGISTER(TZEROWAIT){.sr=0};
		INIT_REGISTER(SW_MODE){{.sr=0}};
		INIT_REGISTER(ENCMODE){{.sr=0}};
		INIT_REGISTER(ENC_CONST){.sr=0};

		struct IFCNT_t 		{ constexpr static uint8_t address = 0x02; }; // R
		struct VACTUAL_t 	{ constexpr static uint8_t address = 0x22; }; // R
		struct XTARGET_t 	{ constexpr static uint8_t address = 0x2D; }; // RW
		struct XLATCH_t 	{ constexpr static uint8_t address = 0x36; }; // R
		struct X_ENC_t 		{ constexpr static uint8_t address = 0x39; }; // RW
		struct ENC_STATUS_t { constexpr static uint8_t address = 0x3B; }; // R+C
		struct ENC_LATCH_t 	{ constexpr static uint8_t address = 0x3C; }; // R

		/*
		INIT_REGISTER(MSLUT0){0};
		INIT_REGISTER(MSLUT1){0};
		INIT_REGISTER(MSLUT2){0};
		INIT_REGISTER(MSLUT3){0};
		INIT_REGISTER(MSLUT4){0};
		INIT_REGISTER(MSLUT5){0};
		INIT_REGISTER(MSLUT6){0};
		INIT_REGISTER(MSLUT7){0};
		INIT_REGISTER(MSLUTSEL){0};
		INIT_REGISTER(MSLUTSTART){0};
		INIT_REGISTER(MSCNT){0};
		INIT_REGISTER(MSCURACT){0};
		*/

		static constexpr float default_RS = 0.15;

	protected:
		using TMC2160Stepper::SHORT_CONF;
		using TMC2160Stepper::s2vs_level;
		using TMC2160Stepper::s2g_level;
		using TMC2160Stepper::shortfilter;
		using TMC2160Stepper::shortdelay;
		using TMC2160Stepper::DRV_CONF;
		using TMC2160Stepper::bbmtime;
		using TMC2160Stepper::bbmclks;
		using TMC2160Stepper::otselect;
		using TMC2160Stepper::drvstrength;
		using TMC2160Stepper::filt_isense;
		using TMC2160Stepper::GLOBAL_SCALER;
		using TMC2160Stepper::OFFSET_READ;

		using TMC2160Stepper::pwm_ofs;
		using TMC2160Stepper::pwm_autograd;
		using TMC2160Stepper::pwm_reg;
		using TMC2160Stepper::pwm_lim;

		using TMC2160Stepper::pwm_scale_sum;
		using TMC2160Stepper::pwm_scale_auto;
};


class TMC5160Stepper : public TMC5130Stepper {
	public:
		TMC5160Stepper(uint16_t pinCS, float RS = default_RS, int8_t link_index = -1);
		TMC5160Stepper(uint16_t pinCS, uint16_t pinMOSI, uint16_t pinMISO, uint16_t pinSCK, int8_t link_index = -1);
		TMC5160Stepper(uint16_t pinCS, float RS, uint16_t pinMOSI, uint16_t pinMISO, uint16_t pinSCK, int8_t link_index = -1);

		void rms_current(uint16_t mA) { TMC2160Stepper::rms_current(mA); }
		void rms_current(uint16_t mA, float mult) { TMC2160Stepper::rms_current(mA, mult); }
		uint16_t rms_current() { return TMC2160Stepper::rms_current(); }
		void defaults();
		void push();

		// RW: GCONF
		void recalibrate(bool);
		void faststandstill(bool);
		void multistep_filt(bool);
		bool recalibrate();
		bool faststandstill();
		bool multistep_filt();

		// R: IOIN
		bool drv_enn() { return drv_enn_cfg6(); }
		bool enc_n_dco_cfg6() { return enc_n_dco(); }

		// W: SHORT_CONF
		using TMC2160Stepper::SHORT_CONF;
		using TMC2160Stepper::s2vs_level;
		using TMC2160Stepper::s2g_level;
		using TMC2160Stepper::shortfilter;
		using TMC2160Stepper::shortdelay;

		// W: DRV_CONF
		using TMC2160Stepper::DRV_CONF;
		using TMC2160Stepper::bbmtime;
		using TMC2160Stepper::bbmclks;
		using TMC2160Stepper::otselect;
		using TMC2160Stepper::drvstrength;
		using TMC2160Stepper::filt_isense;

		// W: GLOBAL_SCALER
		using TMC2160Stepper::GLOBAL_SCALER;

		// R: OFFSET_READ
		using TMC2160Stepper::OFFSET_READ;

		// R+WC: ENC_STATUS
		void ENC_STATUS(uint8_t);
		uint8_t ENC_STATUS();

		// W: ENC_DEVIATION
		void ENC_DEVIATION(uint32_t);
		uint32_t ENC_DEVIATION();

		// R: PWM_AUTO
		uint32_t PWM_AUTO();
		uint8_t pwm_ofs_auto();
		uint8_t pwm_grad_auto();

		// RW: CHOPCONF
		void diss2vs(bool);
		void tpfd(uint8_t);
		bool diss2vs();
		uint8_t tpfd();

		// W: PWM_CONF
		using TMC2160Stepper::PWMCONF;
		using TMC2160Stepper::pwm_ofs;
		using TMC2160Stepper::pwm_grad;
		using TMC2160Stepper::pwm_freq;
		using TMC2160Stepper::pwm_autoscale;
		using TMC2160Stepper::pwm_autograd;
		using TMC2160Stepper::freewheel;
		using TMC2160Stepper::pwm_reg;
		using TMC2160Stepper::pwm_lim;

		using TMC2160Stepper::PWM_SCALE;
		using TMC2160Stepper::pwm_scale_sum;
		using TMC2160Stepper::pwm_scale_auto;

	protected:
		using TMC5130Stepper::I_scale_analog;
		using TMC5130Stepper::internal_Rsense;
		using TMC5130Stepper::enc_commutation;
		using TMC5130Stepper::drv_enn_cfg6;
		using TMC5130Stepper::enc_n_dco;
		using TMC5130Stepper::ENCM_CTRL;
		using TMC5130Stepper::vsense;
		using TMC5130Stepper::rndtf;

		INIT_REGISTER(ENC_DEVIATION){.sr=0};

		static constexpr float default_RS = 0.075;
};

class TMC5161Stepper : public TMC5160Stepper {
	public:
    TMC5161Stepper(uint16_t pinCS, float RS = default_RS, int8_t link_index = -1) : TMC5160Stepper(pinCS, RS, link_index) {}
		TMC5161Stepper(uint16_t pinCS, uint16_t pinMOSI, uint16_t pinMISO, uint16_t pinSCK, int8_t link_index = -1) :
			TMC5160Stepper(pinCS, pinMOSI, pinMISO, pinSCK, link_index) {}
		TMC5161Stepper(uint16_t pinCS, float RS, uint16_t pinMOSI, uint16_t pinMISO, uint16_t pinSCK, int8_t link_index = -1) :
			TMC5160Stepper(pinCS, RS, pinMOSI, pinMISO, pinSCK, link_index) {}
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
