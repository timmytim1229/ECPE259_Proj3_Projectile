%% Project 3
%% Reset the Workspace
close all
clear

%% Setup data transfer/ Aqcuisition
% gravitational acceleration (m/s2)
g = 9.80665;
samplTime = .013;     % Measured

%% Read Data from file
delimiterIn = ',';
headerlinesIn = 1;
%[filename, path] = uigetfile('.txt');
filename = 'sensor_data_handtest2.txt';
%filename = 'sensor_data.txt';
%path = 'C:\git_projects\ECPE259_Proj3_Projectile\project3_graph\SerialTest\';
sensorData = importdata(filename, delimiterIn);

dataSize = length(sensorData.data);
% Specify number of sensors to spilt up data
numberOfSensors = 10;
numVals = ceil(dataSize/numberOfSensors);

% Preallocate 
accel_x = zeros(1, numVals);
accel_y = zeros(1, numVals);
accel_z = zeros(1, numVals);
% temperature = zeros(1, numVals);
gyro_x = zeros(1, numVals);
gyro_y = zeros(1, numVals);
gyro_z = zeros(1, numVals);
quat_w = zeros(1, numVals);
quat_x = zeros(1, numVals);
quat_y = zeros(1, numVals);
quat_z = zeros(1, numVals);

% Read values from specified location
for i = 1:dataSize
    if strcmp(sensorData.textdata(i),'Acceleration_x')
        accel_x(ceil(i/numberOfSensors)) = sensorData.data(i);
    elseif strcmp(sensorData.textdata(i),'Acceleration_y')
        accel_y(ceil(i/numberOfSensors)) = sensorData.data(i);
    elseif strcmp(sensorData.textdata(i),'Acceleration_z')
        accel_z(ceil(i/numberOfSensors)) = sensorData.data(i);
%     elseif strcmp(sensorData.textdata(i),'Temperature')
%         temperature(ceil(i/numberOfSensors)) = sensorData.data(i);
    elseif strcmp(sensorData.textdata(i),'Gyro_x')
        gyro_x(ceil(i/numberOfSensors)) = sensorData.data(i);
    elseif strcmp(sensorData.textdata(i),'Gyro_y')
        gyro_y(ceil(i/numberOfSensors)) = sensorData.data(i);
    elseif strcmp(sensorData.textdata(i),'Gyro_z')
        gyro_z(ceil(i/numberOfSensors)) = sensorData.data(i);
    elseif strcmp(sensorData.textdata(i),'Quat_w')
        quat_w(ceil(i/numberOfSensors)) = sensorData.data(i);
    elseif strcmp(sensorData.textdata(i),'Quat_x')
        quat_x(ceil(i/numberOfSensors)) = sensorData.data(i);
    elseif strcmp(sensorData.textdata(i),'Quat_y')
        quat_y(ceil(i/numberOfSensors)) = sensorData.data(i);
    elseif strcmp(sensorData.textdata(i),'Quat_z')
        quat_z(ceil(i/numberOfSensors)) = sensorData.data(i);
    end
end


%% Define Rotation Matrices
% Define angles from gyro data 
% (defined from North-East-Down [NED] as x-y-z)
% w/ projectile pointing north [http://www.chrobotics.com/library/understanding-euler-angles]

% CHECK: SENSOR DATA IS IN ANGULAR VELOCITY
%%% THE FLLOWING STEPS SHOULD BE DONE PER VALUE OF theta(i), phi(i), AND psi(i)

% Get angles from gyro data
%theta = cumtrapz(gyro_y)*sampleTime;
%phi = cumtrapz(gyro_x)*sampleTime;
%psi = cumtrapz(gyro_z)*sampleTime;

temp = accel_y;
accel_y = accel_x;
accel_x = temp;
accel_z = - accel_z;

accel_y = accel_y(350:end)*g;
accel_x = accel_x(350:end)*g;
accel_z = accel_z(350:end)*g;

temp = gyro_y;
gyro_y = gyro_x;
gyro_x = temp;
gyro_z = -gyro_z;

gyro_x = gyro_x(350:end);
gyro_y = gyro_y(350:end);
gyro_z = gyro_z(350:end);

quat_w = quat_w(350:end);
quat_x = quat_x(350:end);
quat_y = quat_y(350:end);
quat_z = quat_z(350:end);

numVals = numVals - 350;


theta = cumtrapz(gyro_x)*samplTime;
phi = cumtrapz(gyro_y)*samplTime;
psi = cumtrapz(gyro_z)*samplTime;





for i = 1:numVals+1
    % Pitch (rotation about y-axis)
    %theta1(i) = gyro_y(i);
    % Roll (rotation about x-axis)
    %phi1(i) = gyro_x(i);
    % Yaw (rotation about z-axis)
    %psi1(i) = gyro_z(i);
    
    quat = [quat_w(i).^2 + quat_x(i).^2 - quat_y(i).^2 - quat_z(i).^2,...
                    2.*quat_x(i).*quat_y(i) - 2.*quat_w(i).*quat_z(i),...
                    2.*quat_x(i).*quat_z(i) + 2.*quat_x(i).*quat_y(i);
                    
            2.*quat_x(i).*quat_y(i) + 2.*quat_w(i).*quat_z(i),... 
                    quat_w(i).^2 - quat_x(i).^2+quat_y(i).^2 - quat_z(i).^2,... 
                    2.*quat_y(i).*quat_z(i) - 2.*quat_w(i).*quat_x(i);
                    
            2.*quat_x(i).*quat_z(i) - 2.*quat_w(i)*quat_y(i),...
                    2.*quat_y(i).*quat_z(i) + 2.*quat_w(i).*quat_x(i),...
                    quat_w(i).^2 - quat_x(i).^2 - quat_y(i).^2 + quat_z(i).^2];

    iquat = inv(quat);   
    
    % Rotation matrix from inertial to body frame 
    R_ib = [cos(psi(i))*cos(theta(i)), cos(theta(i))*sin(psi(i)), -sin(theta(i));
            cos(psi(i))*sin(phi(i))*sin(theta(i))-cos(phi(i))*sin(psi(i)), ...
            cos(phi(i))*cos(psi(i))+sin(phi(i))*sin(psi(i))*sin(theta(i)), ...
            cos(theta(i))*sin(phi(i));
            sin(phi(i))*sin(psi(i))+cos(phi(i))*cos(psi(i))*sin(theta(i)), ...
            cos(phi(i))*sin(psi(i))*sin(theta(i))-cos(psi(i))*sin(phi(i)), ...
            cos(phi(i))*cos(theta(i))];
    % Rotation matrix from body to inertial frame
    R_bi = [cos(psi(i))*cos(theta(i)), cos(psi(i))*sin(phi(i))*sin(theta(i))-cos(phi(i))*sin(psi(i)),...
            sin(phi(i))*sin(psi(i))+cos(phi(i))*cos(psi(i))*sin(theta(i));
            cos(theta(i))*sin(psi(i)), cos(phi(i))*cos(psi(i))+sin(phi(i))*sin(psi(i))*sin(theta(i)),...
            cos(phi(i))*sin(psi(i))*sin(theta(i))-cos(psi(i))*sin(phi(i));
            -sin(theta(i)), cos(theta(i))*sin(phi(i)), cos(phi(i))*cos(theta(i))];
    % Transformation matrix from body-frame angular rates to Euler angle rates
    D = [1, sin(phi(i))*tan(theta(i)), cos(phi(i))*tan(theta(i));
         0, cos(phi(i)), -sin(phi(i));
         0, sin(phi(i))/cos(theta(i)), cos(phi(i))/cos(theta(i))];
     
    %% Translate Sensor data to inertial (ground) frame
    % Define Motion Vectors (intertial physical accel)
    accel = [accel_x(i); accel_y(i); accel_z(i)];   %1x3
    gyro_m = [gyro_x(i), gyro_y(i), gyro_z(i)];     %1x3
    % Translate measured acceleromoeter data to inertial frame
    %%%a_body(i,:) = (R_ib*accel + [0; 0; g])';
    a_body(i,:) = iquat * accel + [0; 0; g];
    %%a_body = (accel + R_ib*[0; 0; g]);
    %%a_intert(i,:) = (R_bi*a_body + [0; 0; g]);            
    % Translate measured gyroscope data to inertial frame
    %gyro(i,:) = (gyro_m*D).';       % 1x3 * 3x3 = 1x3
    gyro(i,:) = [gyro_x(i) + gyro_y(i).*sin(phi(i)).*tan(theta(i)) + gyro_z(i).*cos(phi(i)).*tan(theta(i)),
                 gyro_y(i).*cos(theta(i)) - gyro_z(i).*sin(phi(i)),
                 gyro_y(i).*sin(phi(i))./cos(theta(i)) + gyro_z(i).*cos(phi(i))./cos(theta(i))].';
end

%theta_inert = cumtrapz(gyro(:,1)).*sampleTime;
%phi_inert = cumtrapz(gyro(:,2)).*sampleTime;
%psi_inert = -cumtrapz(gyro(:,3)).*sampleTime;

%% Calculate Displacement and Plot
t = 0:samplTime:double(numVals)*samplTime;
% Take the integral of the acceleration to get the velocity
%v = cumtrapz(a.*9.81) .* sampleTime;
v = cumtrapz(a_body) .* samplTime;
[P_v,S_v] = polyfit([t;t]',[v(:,1) v(:,3)],2); % fit quadratic function to our velocityd
Y_v = polyval(P_v,t);
% Remove average from data
v(:,1) = v(:,1)-Y_v';
v(:,3) = v(:,3) - Y_v';

% Take the integral of the velocity to get the displacement
%u = cumtrapz(v) .* sampleTime;
u = cumtrapz(v) .* samplTime;

% % Get the index values for each sample
% x_t = (0:numVals - 1)';
% y = u(:,2);
% % Fit a 2nd degree polynomial to the displacement
% p = polyfit(x_t,y, 2);
% % Calculate the values on the polynomial
% y = P(1) .* x_t.^2 + P(2) .*x_t + P(3);
% % Find the difference in the displacement and the polynomial to remove
% % the drift from the data
% u2 = u - y;

% also for unRotated data
v_z = cumtrapz(accel_z) .* samplTime;
u_z = cumtrapz(v_z) .* samplTime;
v_y = cumtrapz(accel_y) .* samplTime;
u_y = cumtrapz(v_y) .* samplTime;
v_x = cumtrapz(accel_x) .* samplTime;
u_x = cumtrapz(v_x) .* samplTime;

% Plot the original Side-displacement - X
figure('Name','UnAdjusted Accel(X) Data','NumberTitle','on');
subplot(3,1,3), plot(t, u_x);
title('Plot of Displacement vs Time (Experimental)')
xlabel('t [s]');
ylabel('u(t) [m]');

subplot(3,1,2), plot(t, v_x);
title('Plot of Velocity vs Time (Experimental)')
xlabel('t [s]');
ylabel('v(t) [m/s]');

subplot(3,1,1), plot(t, accel_x);
title('Plot of Acceleration vs Time (Experimental)')
xlabel('t [s]');
ylabel('a(t) [m s^-2]');

% Plot the original Forward-displacement - Y
figure('Name','UnAdjusted Accel(y) Data','NumberTitle','on');
subplot(3,1,3), plot(t, u_y);
title('Plot of Displacement vs Time (Experimental)')
xlabel('t [s]');
ylabel('u(t) [m]');

subplot(3,1,2), plot(t, v_y);
title('Plot of Velocity vs Time (Experimental)')
xlabel('t [s]');
ylabel('v(t) [m/s]');

subplot(3,1,1), plot(t, accel_y);
title('Plot of Acceleration vs Time (Experimental)')
xlabel('t [s]');
ylabel('a(t) [m s^-2]');

% Plot the original Z-displacement - Z
figure('Name','UnAdjusted Accel(Z) Data','NumberTitle','on');
subplot(3,1,3), plot(t, u_z);
title('Plot of Displacement vs Time (Experimental)')
xlabel('t [s]');
ylabel('u(t) [m]');

subplot(3,1,2), plot(t, v_z);
title('Plot of Velocity vs Time (Experimental)')
xlabel('t [s]');
ylabel('v(t) [m/s]');

subplot(3,1,1), plot(t, accel_z);
title('Plot of Acceleration vs Time (Experimental)')
xlabel('t [s]');
ylabel('a(t) [m s^-2]');




% Plot the adjusted displacement - X
figure('Name','Rotated Accel(X) Data','NumberTitle','on');
subplot(3,1,3), plot(t, u(:,1));
title('Plot of Displacement vs Time (Experimental)')
xlabel('t [s]');
ylabel('u(t) [m]');

subplot(3,1,2), plot(t, v(:,1));
title('Plot of Velocity vs Time (Experimental)')
xlabel('t [s]');
ylabel('v(t) [m/s]');

subplot(3,1,1), plot(t, a_body(:,1));
title('Plot of Acceleration vs Time (Experimental)')
xlabel('t [s]');
ylabel('a(t) [m s^-2]');

% Plot the adjusted displacement - Y
figure('Name','Rotated Accel(Y) Data','NumberTitle','on');
subplot(3,1,3), plot(t, u(:,2));
title('Plot of Displacement vs Time (Experimental)')
xlabel('t [s]');
ylabel('u(t) [m]');

subplot(3,1,2), plot(t, v(:,2));
title('Plot of Velocity vs Time (Experimental)')
xlabel('t [s]');
ylabel('v(t) [m/s]');

subplot(3,1,1), plot(t, a_body(:,2));
title('Plot of Acceleration vs Time (Experimental)')
xlabel('t [s]');
ylabel('a(t) [m s^-2]');

% Plot the adjusted displacement - Z
figure('Name','Rotated Accel(Z) Data','NumberTitle','on');
subplot(3,1,3), plot(t, u(:,3));
title('Plot of Displacement vs Time (Experimental)')
xlabel('t [s]');
ylabel('u(t) [m]');

subplot(3,1,2), plot(t, v(:,3));
title('Plot of Velocity vs Time (Experimental)')
xlabel('t [s]');
ylabel('v(t) [m/s]');

subplot(3,1,1), plot(t, a_body(:,3));
title('Plot of Acceleration vs Time (Experimental)')
xlabel('t [s]');
ylabel('a(t) [m s^-2]');



% Plot the Gyro Data
figure('Name','UnAdjusted Gyro Data','NumberTitle','on');
subplot(3,1,1), plot(t, gyro_x);
title('Plot of Roll Angle vs Time (Experimental)')
xlabel('t [s]');
ylabel('w_{\phi} [rad/s]');

subplot(3,1,2), plot(t, gyro_y);
title('Plot of Pitch Angle vs Time (Experimental)')
xlabel('t [s]');
ylabel('w_{\theta} [rad/s]');

subplot(3,1,3), plot(t, gyro_z);
title('Plot of Yaw Angle vs Time (Experimental)')
xlabel('t [s]');
ylabel('w_{\psi} [rad/s]');

% Plot the adjusted Gyro Data
figure('Name','Rotated Gyro Data','NumberTitle','on');
subplot(3,1,1), plot(t, gyro(:,1));
title('Plot of Roll Angle vs Time (Experimental)')
xlabel('t [s]');
ylabel('w_{\phi} [rad/s]');

subplot(3,1,2), plot(t, gyro(:,2));
title('Plot of Pitch Angle vs Time (Experimental)')
xlabel('t [s]');
ylabel('w_{\theta} [rad/s]');

subplot(3,1,3), plot(t, gyro(:,3));
title('Plot of Yaw Angle vs Time (Experimental)')
xlabel('t [s]');
ylabel('w_{\psi} [rad/s]');

%% Calculate Distance From Theoretical Results
%Conversion factors
ft_m = 0.3048; 
deg_rad = pi/180;
% Initial Conditions
x_o = 0;
y_o = 4*ft_m;
% Launch angle
% angle = 28.6*deg_rad;
angle = 38.7*deg_rad; %% add user input???
% angle = 52.9*deg_rad;
% Mass (kg)
m = 0.16;
% Sring constant (kg/s^2)
k = 127.35;
% Total pullback distance (d)
d = 3*ft_m;
ratio = 0.5; %% add user input???
s = d*ratio;
v_o = sqrt(2*g*s*sin(angle)+(4/m)*(.5*k*s^2));
t_air = (-v_o-sqrt(v_o^2-2*g*y_o))/g;
t_g = 2.4472;
%t_air = 2*v_o*sin(angle)/g;
t_p = 0:samplTime:t_g;
% Theoretical Motion Eqns
y = y_o + v_o*sin(angle).*t_p -.5*g*t_p.^2;
x = v_o*t_p;

%% Find Distance from data


%% Error

