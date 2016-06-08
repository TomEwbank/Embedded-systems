
bool send_ping
bool ping_send
bool blocking_sensors
bool listening

int 1ms_counter = 0;

bool sensor1_turn = true;

bool RTT_received = false;
bool RTT1_received = false;
bool RTT2_received = false;

char no_RTT1_counter = false; 
char no_RTT2_counter = false;

char sensor1_RTT_low = 0;
char sensor1_RTT_high = 0;

int sensor2_RTT_low = 0;
int sensor2_RTT_high = 0;

// State handling (ping sending, sensor blocking, input listening)
interrupt timer0() // every 1ms (1kHz)
{
    if (listening) {
        if (1ms_counter == 30) {
            1ms_counter = 0;
            send_ping = 1;
            
            if (sensor1_turn) {
                if (RTT1_received) {
                    no_RTT1_counter = 0;
                } else {
                    ++no_RTT1_counter;
                }
            } else {
                if (RTT2_received) {
                    no_RTT2_counter = 0;
                } else {
                    ++no_RTT2_counter;
                }
            }

            RTT_received = false;
            sensor1_turn = !sensor1_turn;
            ADIF = 0;
            ADIE = 0;
        } else {
            ++1ms_counter;
        }
    } else if (ping_send) { 
        stopPWM();
        ping_send = 0;
        blocking_sensors = 1;
    } else if (send_ping) {
        TMR3L = 0;
        TMR3H = 0;
        startTimer1();
        startPWM(); // Sent for 1ms
        ping_send = 1;
    } else if (blocking_sensors && 1ms_counter == 15) { // Blocked during 15ms
        1ms_counter = 0;
        blocking_sensors = 0;
        listening = 1;
        ADIE = 1;
        launch_ADC_conversion();
    } else {
        ++1ms_counter;
    }

}

interrupt adc_recup_value() // Maybe need higher priority
{
    if (sensor1_turn) {
        int sensor1_val = get_ADC_value();

        if (sensor1_val > threshold_high || sensor2_val < threshold_low) {
            sensor1_RTT_low = TMR3L;
            sensor1_RTT_high = TMR3H;
            stopTimer1();
            RTT1_received = true;
            RTT_received = true;
            ADIE = 0;
        }

    } else {
        int sensor2_val = get_ADC_value();

        if (sensor2_val > threshold_high || sensor2_val < threshold_low) {
            sensor2_RTT_low = TMR3L;
            sensor2_RTT_high = TMR3H;
            stopTimer1();
            RTT2_received = true;
            RTT_received = true;
            ADIE = 0;
        }
    }
    launch_ADC_conversion();
}


void main() {
    initOscillo();
    initTimer0();
    initTimer1(); // RTT_timer
    initPWM();
    initUART();

    int fetched_sensor1_RTT = 0;
    int fetched_sensor2_RTT = 0;

    while(1) {

        if (no_RTT1_counter > 3 || no_RTT2_counter > 3) {
            send_debug("Tracker not found");
        } else {
            // RTT fetching
            if (RTT_received) {
                fetched_sensor1_RTT = (sensor1_RTT_high<<8) & sensor1_RTT_low;
                fetched_sensor2_RTT = (sensor1_RTT_high<<8) & sensor1_RTT_low;
            }

            send_coord(fetched_sensor1_RTT, fetched_sensor2_RTT);
            // Coordinates calculation
            // Point p;
            // track(fetched_sensor1_RTT, fetched_sensor2_RTT, &p);

            // UART sending
            // send_coord(p.x,p.y);
    }
}