%% Project 3

%%
% Take the integral of the acceleration to get the velocity
vs = cumtrapz(gs.*9.81) .* sampleRate;
% Take the integral of the velocity to get the displacement
us = cumtrapz(vs) .* sampleRate;
% Get the index values for each sample
x = (0:numSamples - 1)';
% Fit a 2nd degree polynomial to the displacement
P = polyfit(x, us, 2);
% Calculate the values on the polynomial
y = P(1) .* x.^2 + P(2) .* x + P(3);
% Find the difference in the displacement and the polynomial to remove
% the drift from the data
us2 = us - y;
% Plot the displacement
figure;
plot(t, us2);
title('Plot of Displacement vs Time (Experimental)')
xlabel('t [s]');
ylabel('u(t) [m]');

%%
delimiterIn = ',';
%headerlinesIn = 2;
filename = 'sensor_data.txt';
A = importdata(filename, delimiterIn)

dataSize = length(A.data)
numberOfSensors = 7

% Preallocate 
accel_x = zeros(1, int32(dataSize/numberOfSensors)+1);
accel_y = zeros(1, int32(dataSize/numberOfSensors)+1);
accel_z = zeros(1, int32(dataSize/numberOfSensors)+1);
temperature = zeros(1, int32(dataSize/numberOfSensors)+1);
gyro_x = zeros(1, int32(dataSize/numberOfSensors)+1);
gyro_y = zeros(1, int32(dataSize/numberOfSensors)+1);
gyro_z = zeros(1, int32(dataSize/numberOfSensors)+1);

for i = 1:dataSize
    if strcmp(A.textdata(i),'Acceleration_x')
        accel_x(ceil(i/7)) = A.data(i);
    elseif strcmp(A.textdata(i),'Acceleration_y')
        accel_y(ceil(i/7)) = A.data(i);
    elseif strcmp(A.textdata(i),'Acceleration_z')
        accel_z(ceil(i/7)) = A.data(i);
    elseif strcmp(A.textdata(i),'Temperature')
        temperature(ceil(i/7)) = A.data(i);
    elseif strcmp(A.textdata(i),'Gyro_x')
        gyro_x(ceil(i/7)) = A.data(i);
    elseif strcmp(A.textdata(i),'Gyro_y')
        gyro_y(ceil(i/7)) = A.data(i);
    elseif strcmp(A.textdata(i),'Gyro_z')
        gyro_z(ceil(i/7)) = A.data(i);
    end
        
end


