delay = 10/40000;
v_adjust = 0; 

x = -1:1:1;
y = [3,2,1,0.1];

v = 343; % speed of sound
x_captors = 0.03; 
a = x_captors*2;

t_captor1 = zeros(length(y),length(x));
t_captor2 = zeros(length(y),length(x));
r = zeros(length(y),length(x));
x_approx = zeros(length(y),length(x));
x_approx_bis = zeros(length(y),length(x));
y_approx = zeros(length(y),length(x));
delta_alpha = zeros(length(y),length(x));
delta_d = zeros(length(y),length(x));

for i=1:length(x)
    for j=1:length(y)
       
        d0 = sqrt(x(i)^2+y(j)^2);
        d10 = sqrt((x(i)+x_captors)^2+y(j)^2);
        d20 = sqrt((x(i)-x_captors)^2+y(j)^2);
        t_captor1(j,i) = (d0+d10)/(v+v_adjust);
        t_captor2(j,i) = (d0+d20)/(v+v_adjust);
        
%         d1 = d0+d10;
%         d2 = d0+d20;
        d1 = (t_captor1(j,i)+delay)*v;
        d2 = (t_captor2(j,i)+delay)*v;
        e = x_captors;
        
        if d1 == d2
            x_approx(j,i) = 0;
            y_approx(j,i) = (d1^2-e^2)/(2*d1);
        else
            m = e^2-d1^2;
            n = e^2-d2^2;
            a = (n/d2^2)-(m/d1^2);
            b = -e*((n/d2^2)+(m/d1^2));
            c = (n^2/(4*d2^2))-(m^2/(4*d1^2));
            
            delta = b^2-4*a*c;
            
            x_approx(j,i) = (-b+sqrt(delta))/(2*a);
            y_approx(j,i) = sqrt((m*x_approx(j,i)^2+m*e*x_approx(j,i)+m^2/4)/d1^2);        
        end
        
                
%         AD = abs(t_captor1(j,i)-t_captor2(j,i))*v
%         cos_theta = AD/(2*x_captors)
%         DB = sqrt(AD^2+4*x_captors^2-4*x_captors*AD*cos_theta)
%         ADB = acos((DB^2+AD^2-4*x_captors^2)/(2*DB*AD))
%         phi = pi-ADB
%         r(j,i) = DB/(2*cos(phi))
%         AC = r(j,i)+AD
%         MC = sqrt(AC^2+x_captors^2-2*AC*x_captors*cos_theta)
%         cos_alpha = (MC^2+x_captors^2-r(j,i)^2)/(2*MC*x_captors)
%         sin_alpha = sqrt(1-cos_alpha^2)
%         
%         x_approx(j,i) = cos_alpha*MC;
%         y_approx(j,i) = sin_alpha*MC;
%         
%         delta_alpha(j,i) = abs(acos(cos_alpha)-atan(y(j)/x(i)));
         delta_d(j,i) = sqrt((x(i)-x_approx(j,i))^2+(y(j)-y_approx(j,i))^2);
    end   
end





