clc
close all
clear all

d = [200,5.04; 400,11.16; 800,23.38; 1000,29.48; 2000,60];
t = d(:,1);
y = d(:,2)*1e-3;

p = polyfit(t,y,1);
fx = [0,1.1*max(t)];
fy = polyval(p,fx);

figure
plot(t,y,'ro')
hold on
plot(fx,fy,'b:')
xlim([0,Inf])

fprintf('%.3f us/step\n',p(1)/1e-6)
delay = 100e-3;
fprintf('need %d steps for %.2f ms\n',round(delay/p(1)),delay/1e-3)

