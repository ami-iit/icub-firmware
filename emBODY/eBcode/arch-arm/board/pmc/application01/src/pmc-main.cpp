
/*
 * Copyright (C) 2019 iCub Tech - Istituto Italiano di Tecnologia
 * Author:  Marco Accame
 * email:   marco.accame@iit.it
*/


#include "embot_app_skeleton_os_evthreadcan.h"

// --------------------------------------------------------------------------------------------------------------------
// config start

constexpr embot::app::theCANboardInfo::applicationInfo applInfo 
{ 
    embot::prot::can::versionOfAPPLICATION {1, 0, 0},    
    embot::prot::can::versionOfCANPROTOCOL {2, 0}    
};

constexpr std::uint16_t threadIDLEstacksize = 2048;
constexpr std::uint16_t threadINITstacksize = 2048;
constexpr std::uint16_t threadEVNTstacksize = 4096;
constexpr std::uint8_t maxINPcanframes = 16;
constexpr std::uint8_t maxOUTcanframes = 48;
constexpr embot::core::relTime threadEVNTtimeout = 50*embot::core::time1millisec;

static void *paramINIT = nullptr;
static void *paramIDLE = nullptr;
static void *paramERR = nullptr;
static void *paramEVNT = nullptr;

constexpr embot::os::theTimerManager::Config tmcfg {};
constexpr embot::os::theCallbackManager::Config cmcfg {};
    
    
static const embot::app::skeleton::os::basic::sysConfig syscfg { threadINITstacksize, paramINIT, threadIDLEstacksize, paramIDLE, paramERR, tmcfg, cmcfg};

static const embot::app::skeleton::os::evthreadcan::evtConfig evtcfg { threadEVNTstacksize, paramEVNT, threadEVNTtimeout};

static const embot::app::skeleton::os::evthreadcan::canConfig cancfg { maxINPcanframes, maxOUTcanframes };

// config end
// --------------------------------------------------------------------------------------------------------------------


class mySYS final : public embot::app::skeleton::os::evthreadcan::SYSTEMevtcan
{
public:
    mySYS(const embot::app::skeleton::os::basic::sysConfig &cfg) 
        : SYSTEMevtcan(cfg) {}
        
    void userdefOnIdle(embot::os::Thread *t, void* idleparam) const override;
    void userdefonOSerror(void *errparam) const override;
    void userdefInit_Extra(embot::os::EventThread* evthr, void *initparam) const override;
};


class myEVT final : public embot::app::skeleton::os::evthreadcan::evThreadCAN
{
public:
    myEVT(const embot::app::skeleton::os::evthreadcan::evtConfig& ecfg, const embot::app::skeleton::os::evthreadcan::canConfig& ccfg, const embot::app::theCANboardInfo::applicationInfo& a) 
        : evThreadCAN(ecfg, ccfg, a) {}
        
    void userdefStartup(embot::os::Thread *t, void *param) const override;
    void userdefOnTimeout(embot::os::Thread *t, embot::os::EventMask eventmask, void *param) const override;
    void userdefOnEventRXcanframe(embot::os::Thread *t, embot::os::EventMask eventmask, void *param, const embot::prot::can::Frame &frame, std::vector<embot::prot::can::Frame> &outframes) const override;
    void userdefOnEventANYother(embot::os::Thread *t, embot::os::EventMask eventmask, void *param, std::vector<embot::prot::can::Frame> &outframes) const override;                   
};


static const mySYS mysys { syscfg };
static const myEVT myevt { evtcfg, cancfg, applInfo };
constexpr embot::app::skeleton::os::evthreadcan::CFG cfg{ &mysys, &myevt };

// --------------------------------------------------------------------------------------------------------------------

int main(void)
{ 
    embot::app::skeleton::os::evthreadcan::run(cfg);
    for(;;);    
}


// - here is the tailoring of the board.

#define enableTRACE_all


#include "embot_os_theScheduler.h"
#include "embot_app_theLEDmanager.h"
#include "embot_app_application_theCANparserBasic.h"
#include "embot_app_application_theCANtracer.h"

#include "embot_hw_tlv493d.h"
#include "embot_hw_bsp.h"
#include "embot_hw_gpio.h"
#include "embot_hw_can.h"


constexpr embot::os::Event evtTick = embot::core::binary::mask::pos2mask<embot::os::Event>(2);
constexpr embot::core::relTime tickperiod = 2000*embot::core::time1millisec;

constexpr embot::os::Event evtReadFAP = embot::core::binary::mask::pos2mask<embot::os::Event>(3);
embot::hw::tlv493d::Position positionFAP = 0;


void alertFAPdataisready(void *p)
{
    embot::os::Thread *t = reinterpret_cast<embot::os::Thread*>(p);    
    t->setEvent(evtReadFAP);     
}


void mySYS::userdefOnIdle(embot::os::Thread *t, void* idleparam) const
{
    static int a = 0;
    a++;        
}

void mySYS::userdefonOSerror(void *errparam) const
{
    static int code = 0;
    embot::os::theScheduler::getInstance().getOSerror(code);
    for(;;);    
}

constexpr embot::hw::GPIO VPPEN {embot::hw::GPIO::PORT::C, embot::hw::GPIO::PIN::four};
bool VPPENon {true};
void VPPEenable(bool enable)
{
    embot::hw::gpio::set(VPPEN, (true == enable) ? embot::hw::gpio::State::SET : embot::hw::gpio::State::RESET); 
}

constexpr embot::hw::GPIO MAGVCC2 {embot::hw::GPIO::PORT::E, embot::hw::GPIO::PIN::eleven};

void IndexAdductionEnable(bool enable)
{
    embot::hw::gpio::set(MAGVCC2, (true == enable) ? embot::hw::gpio::State::SET : embot::hw::gpio::State::RESET); 
}


void toggleVPPEN(void *p)
{    
    if(true == VPPENon)
    {
        VPPENon = false;
        VPPEenable(false);
        embot::hw::led::off(embot::hw::LED::seven);        
    }
    else
    {
        VPPENon = true;
        VPPEenable(true);
        embot::hw::led::on(embot::hw::LED::seven);
    }
    
}

void mySYS::userdefInit_Extra(embot::os::EventThread* evthr, void *initparam) const
{
    // inside the init thread: put the init of many things ...  
    
    // led manager
    static const std::initializer_list<embot::hw::LED> alltheleds = { 
        embot::hw::LED::one, embot::hw::LED::two, embot::hw::LED::three, embot::hw::LED::four, 
        embot::hw::LED::five, embot::hw::LED::six, embot::hw::LED::seven
    };  
    embot::app::theLEDmanager &theleds = embot::app::theLEDmanager::getInstance();     
    theleds.init(alltheleds);   
    
    theleds.get(embot::hw::LED::one).pulse(embot::core::time1second); 
//    theleds.get(embot::hw::LED::two).on(); 
//    theleds.get(embot::hw::LED::three).on(); 
//    theleds.get(embot::hw::LED::four).on();
//    theleds.get(embot::hw::LED::five).pulse(2*embot::core::time1second);  
//          
    embot::app::LEDwaveT<64> ledwave(100*embot::core::time1millisec, 30, std::bitset<64>(0b000001));
    theleds.get(embot::hw::LED::six).wave(&ledwave);     

//    theleds.get(embot::hw::LED::seven).pulse(3*embot::core::time1second);    

    
    // init of can basic paser
    embot::app::application::theCANparserBasic::getInstance().initialise({});
        
    
                   
//    // init agent of imu
//    embot::app::application::theIMU &theimu = embot::app::application::theIMU::getInstance();
//    embot::app::application::theIMU::Config configimu(embot::hw::bsp::strain2::imuBOSCH, embot::hw::bsp::strain2::imuBOSCHconfig, evIMUtick, evIMUdataready, evthr);
//    theimu.initialise(configimu);

//    // init canparser imu and link it to its agent
//    embot::app::application::theCANparserIMU &canparserimu = embot::app::application::theCANparserIMU::getInstance();
//    embot::app::application::theCANparserIMU::Config configparserimu { &theimu };
//    canparserimu.initialise(configparserimu);   
        
    
    // pc4 (VPPEN)
    // init pc4 as output  (already configured) 
    // start a timer which toggles pc4.
        
        
    embot::hw::led::off(embot::hw::LED::seven);
    VPPEenable(false);
    VPPENon = false;       
        
    embot::os::Timer *tmrVPPEN = new embot::os::Timer;   
    embot::os::Action actVPPEN(embot::os::CallbackToThread(embot::core::Callback{toggleVPPEN, nullptr}, nullptr));
    embot::os::Timer::Config cfg{10*embot::core::time1second, actVPPEN, embot::os::Timer::Mode::forever, 0};
//    tmrVPPEN->start(cfg);      

// pe11 MAGVCC2 high 

//    IndexAdductionEnable(false);
//    embot::core::delay(1000*1000);

//    
          
    
}

constexpr embot::hw::TLV493D magencIndexAdduction = embot::hw::TLV493D::four;

void myEVT::userdefStartup(embot::os::Thread *t, void *param) const
{
    // inside startup of evnt thread: put the init of many things ... 
    
    // init the fap
    
    embot::core::print("userdefStartup(): called");
    
    embot::core::print("userdefStartup(): start a timer which sends an event which forces an acquisition from the FAP");
    
//    embot::hw::tlv493d::init(magencIndexAdduction, {{embot::hw::bsp::tlv493d::getBSP().getPROP(magencIndexAdduction)->i2cbus, embot::hw::i2c::Speed::standard100}});
    
    // start a timer which sends an event which forces an acquisition from the FAP  
    embot::os::Timer *tmr = new embot::os::Timer;   
    embot::os::Action act(embot::os::EventToThread(evtTick, t));
    embot::os::Timer::Config cfg{tickperiod, act, embot::os::Timer::Mode::forever, 0};
    tmr->start(cfg);     
 
    // maybe we start the tx of ft data straight away
//#if defined(DEBUG_atstartup_tx_FTdata)
//    embot::app::application::theSTRAIN &thestrain = embot::app::application::theSTRAIN::getInstance();
//    thestrain.setTXperiod(10*embot::core::time1millisec);
//    thestrain.start(embot::prot::can::analog::polling::Message_SET_TXMODE::StrainMode::txUncalibrated);
//#endif      
}


void myEVT::userdefOnTimeout(embot::os::Thread *t, embot::os::EventMask eventmask, void *param) const
{
    static uint32_t cnt = 0;
    cnt++;    
}


void myEVT::userdefOnEventRXcanframe(embot::os::Thread *t, embot::os::EventMask eventmask, void *param, const embot::prot::can::Frame &frame, std::vector<embot::prot::can::Frame> &outframes) const
{        
    // process w/ the basic parser. if not recognised call the parsers specific of the board
    if(true == embot::app::application::theCANparserBasic::getInstance().process(frame, outframes))
    {                   
    }
//    else if(true == embot::app::application::theCANparserIMU::getInstance().process(frame, outframes))
//    {               
//    }

}

#if 0
    void transmit()
    {
        static uint8_t cnt {0};
        static embot::hw::can::Frame frame {};
        frame.id = 0x201;
        frame.size = 2;
        frame.data[0] = 0x66;
        frame.data[1] = cnt++;
        embot::hw::can::put(embot::hw::CAN::one, frame);
        embot::hw::can::transmit(embot::hw::CAN::one);            
    }
#endif

void myEVT::userdefOnEventANYother(embot::os::Thread *t, embot::os::EventMask eventmask, void *param, std::vector<embot::prot::can::Frame> &outframes) const
{
    
    static embot::core::Time startOfFAPacquisition = 0;
    
    if(true == embot::core::binary::mask::check(eventmask, evtTick)) 
    {
        startOfFAPacquisition = embot::core::now();
#if defined(enableTRACE_all)        
        embot::core::TimeFormatter tf(startOfFAPacquisition);        
        embot::core::print("userdefOnEventANYother() -> evtTick received @ time = " + tf.to_string());    
#endif 

//        transmit();       

//        embot::core::print("::userdefOnEventANYother() -> called a reading of chip TLV493D on fap board");    
//        embot::core::Callback cbk00(alertFAPdataisready, t);
//        embot::hw::tlv493d::acquisition(magencIndexAdduction, cbk00);        
    }
    
    if(true == embot::core::binary::mask::check(eventmask, evtReadFAP)) 
    {
        embot::core::Time tnow = embot::core::now();
#if defined(enableTRACE_all)        
        embot::core::TimeFormatter tf(tnow);        
        embot::core::print("userdefOnEventANYother() -> evtReadFAP received @ time = " + tf.to_string() + 
                           " acquisition time = " + embot::core::TimeFormatter(tnow-startOfFAPacquisition).to_string());    
#endif  

        if(embot::hw::resOK != embot::hw::tlv493d::read(magencIndexAdduction, positionFAP))
        {
            positionFAP = 66666;
        }
        
        embot::core::print("FAP pos = " + std::to_string(0.01 * positionFAP) + "deg");        
    }      
    
//    if(true == embot::core::binary::mask::check(eventmask, evIMUtick))
//    {        
//        embot::app::application::theIMU &theimu = embot::app::application::theIMU::getInstance();
//        theimu.tick(outframes);        
//    }   
//    
//    if(true == embot::core::binary::mask::check(eventmask, evIMUdataready))
//    {        
//        embot::app::application::theIMU &theimu = embot::app::application::theIMU::getInstance();
//        theimu.processdata(outframes);        
//    }
     
    
}




// - end-of-file (leave a blank line after)----------------------------------------------------------------------------



