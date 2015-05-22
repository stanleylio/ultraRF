clc
close all
clear all

D = importfile('serial_cap.txt');
t = D(:,1);
y = D(:,2);

y = y/4096*2.5;

numel(y)

figure
plot(t,y,'r.-')
xlabel('sample index')
ylabel('ADC0, volt (? not sure, but doesn''t matter)')

