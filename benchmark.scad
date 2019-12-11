cube([23, 20, 2]);
translate([5, 5, 2]) difference() {
    cylinder(h=8, r=2.9, $fn=90);
    cylinder(h=9, r=1.4, $fn=70); 
}
translate([10, 0, 2]) difference() {
    cube([10, 10, 8]);
    translate([1.6, 1.6, 0]) cube([6.8, 6.8, 9]);
}
translate([20, 0, 2]) cube([3, 10, 4]);
translate([11.5, 15, 2])  scale([1.5,.5]) difference() {
    cylinder(h=8, r=7.5, $fn=120);
    cylinder(h=9, r=5.5, $fn=120);
}