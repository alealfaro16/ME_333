function data = client(port)
%   provides a menu for accessing PIC32 motor control functions
%
%   client(port)
%
%   Input Arguments:
%       port - the name of the com port.  This should be the same as what
%               you use in screen or putty in quotes ' '
%
%   Example:
%       client('/dev/ttyUSB0') (Linux/Mac)
%       client('COM3') (PC)
%
%   For convenience, you may want to change this so that the port is hardcoded.
   
% Opening COM connection
if ~isempty(instrfind)
    fclose(instrfind);
    delete(instrfind);
end

fprintf('Opening port %s....\n',port);

% settings for opening the serial port. baud rate 230400, hardware flow control
% wait up to 120 seconds for data before timing out
mySerial = serial(port, 'BaudRate', 230400, 'FlowControl', 'hardware','Timeout',120); 
% opens serial connection
fopen(mySerial);
% closes serial port when function exits
clean = onCleanup(@()fclose(mySerial));                                 

has_quit = false;
% menu loop
while ~has_quit
    fprintf('PIC32 MOTOR DRIVER INTERFACE\n\n');
    % display the menu options; this list will grow
    fprintf(' a: Read current sensor (ADC counts) b:Read current sensor (mA) c: Read encoder (counts) \n d: Read encoder (deg) e: Reset Encoder f: Set PWM (-100 to 100) \n g:Set Current Gains h: Get Current Gains i: Set Position Gains\n j: Get Position Gains k: ITEST l: HOLD \n m: Load Step Trajectory n: Load Cubic Trajectory o: Execute Trajectory \n p: Unpower motor q: Quit r: Get Mode \n');
    % read the user's choice
    selection = input('\nENTER COMMAND: ', 's');
     
    % send the command to the PIC32
    fprintf(mySerial,'%c\n',selection);
    
    % take the appropriate action
    switch selection
        case 'a'
            adc_read  = fscanf(mySerial,'%d');
            fprintf('The current is %d \n (ADC counts)',adc_read);
        case 'b'
            adc_read  = fscanf(mySerial,'%d');
            fprintf('The current is %d mA\n',adc_read); 
        case 'c'
            counts = fscanf(mySerial,'%d');   % get the count from the coder
            fprintf('The motor angle is %d\n',counts);     % print it to the screen
        case 'd'                         % example operation
            counts = fscanf(mySerial,'%d');   % get the count from the coder
            fprintf('The motor angle is %3.2f\n',counts);     % print it to the screen
        case 'e'
            counts = fscanf(mySerial,'%d');   % get the count from the coder
            fprintf('The motor angle is reset to %d\n',counts);  % print it to the screen
        case 'f'
            fprintf('Select duty cycle between -100 and 100');
            duty_cycle = input('\nEnter duty cycle: ');
            fprintf(mySerial,'%d\n',duty_cycle);
            PR_value = fscanf(mySerial, '%d');
            fprintf('The PWM PR value is %d\n',PR_value);
        case 'g'
            fprintf('Set current gains (Kp Ki):');
            KpC = input('\nEnter Kp: ');
            fprintf(mySerial,'%d\n',KpC);
            KiC = input('\nEnter Ki: ');
            fprintf(mySerial,'%d\n',KiC);
        case 'h'
            c_gains = fscanf(mySerial,'%d %d');
            fprintf('Kp is %d and Ki is %d\n',c_gains(1),c_gains(2));
        case 'i'
            fprintf('Set position gains (Kp Ki Kd):');
            KpP = input('\nEnter Kp: ');
            fprintf(mySerial,'%d\n',KpP);
            KiP = input('\nEnter Ki: ');
            fprintf(mySerial,'%d\n',KiP);
            KdP = input('\nEnter Kd: ');
            fprintf(mySerial,'%d\n',KdP);
        case 'j'
            p_gains = fscanf(mySerial,'%d %d');
            fprintf('Kp is %d and Ki is %d Kd is %d\n',p_gains(1),p_gains(2),p_gains(3));
        case 'k'
            fprintf('Ongoing ITEST');
            read_plot_matrix(mySerial);
        case 'l'
            fprintf('Set desired angle (deg)');
            theta = input('\nEnter angle:');
            fprintf(mySerial, '%d\n',theta);
            fprintf('Trying to hold %d degrees',theta);
            read_plot_matrix(mySerial);
        case 'm'
            fprintf('Set the desired positions and times:');
            reflist = input('\nEnter reflist:');
            ref = genRef(reflist,'step');
            [~,len] = size(ref);
            fprintf(mySerial, '%d\n',len);
            for i = 1:len
                fprintf(mySerial, '%d\n',ref(i));
            end
        case 'n'
            fprintf('Set the desired positions and times:');
            reflist = input('\nEnter reflist:');
            ref = genRef(reflist,'cubic');
            [~,len] = size(ref);
            fprintf(mySerial, '%d\n',len);
            for i = 1:len
                fprintf(mySerial, '%3.2f\n',ref(i));
            end
        case 'o'
            fprintf('Trying to follow the trajectory');
            data = read_plot_matrix_deg(mySerial);
        case 'p'
            fprintf('The Motor is off\n')
        case 'q'
            has_quit = true;             % exit client
        case 'r'
            fprintf('0: IDLE 1: PWM 2: ITEST 3: HOLD 4:TRACK\n');
            mode = fscanf(mySerial, '%d');
            fprintf('Current mode is: %d\n',mode);
        otherwise
            fprintf('Invalid Selection %c\n', selection);
    end
end

end
