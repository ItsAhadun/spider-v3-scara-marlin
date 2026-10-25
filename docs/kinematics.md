# HALO kinematics for Marlin

HALO ships no firmware config. We measured every number below from the STL files in [koaldesigns/HALO](https://github.com/koaldesigns/HALO) (`main` branch) and checked them against HALO's BOM.

## Drivetrain

Both arm motors sit in the base. Each motor turns a GT2 16T pulley, and a 288 mm GT2 loop carries the motion to a 128T pulley (DriveGear.stl) on a 7 mm hex shaft. The shaft's 11T pinion (CorePinion) meshes with a 42T internal ring gear.

- **Shoulder:** the ring gear is part of ProximalArm.stl.
- **Elbow:** the ring gear is part of DistalDriveGear.stl. That part sits on the shoulder axis and also carries a 169T GT2 pulley. An 800 mm belt runs from it to a 122T pulley (DistalConnectorGear.stl) fixed to the distal arm at the elbow.

Reduction per joint: 128/16 × 42/11 = 30.545.

| Part | Measurement | Teeth |
|---|---|---|
| DriveGear | tooth profile r 39.74 to 40.49 mm | 128 |
| CorePinion | r 10.1 to 12.1 mm | 11 |
| Ring gear (ProximalArm, DistalDriveGear) | r 41 to 43 mm | 42 |
| DistalDriveGear pulley | OD r 53.54 mm, pitch diameter 107.6 mm | 169 |
| DistalConnectorGear | OD r 38.58 mm, pitch diameter 77.67 mm | 122 |

GT2 has a 2 mm pitch, so teeth = π × pitch diameter / 2.

Two cross-checks:

- A 169T and a 122T pulley 255 mm apart need an 801.9 mm belt. HALO's BOM lists an 800 mm loop.
- The pinion axis sits 31 mm from the joint axis. BaseMotors.stl has its bearing bores at r = 31.09 mm.

## Crosstalk factor

Marlin sets motor B to `psi + theta × SCARA_CROSSTALK_FACTOR` (`Marlin/src/module/scara.cpp`, `inverse_kinematics`). Theta is the shoulder angle. Psi is the elbow angle measured from the proximal arm.

Call the angle of DistalDriveGear D. The belt turns the elbow pulley, relative to the arm, by (D − θ) × 169/122:

```
ψ = (D − θ) × 169/122
D = ψ × 122/169 + θ
```

Motor B turns 30.545 × D. A 1.8° motor at 16 microsteps makes 8.889 steps per motor degree, so in steps:

```
B = 271.52 × D = 196.01 × ψ + 271.52 × θ
```

Marlin's form is `B = 196.01 × (ψ + c × θ)`, which gives c = 271.52 / 196.01 = 169/122 ≈ 1.385.

Hold motor B and turn the shoulder by Δ. The distal arm turns −1.385Δ relative to the proximal arm and −0.385Δ relative to the base. HALO fits neither common value: 0 describes an elbow motor riding on the arm, and 1 describes the Morgan and MPSCARA layouts.

## Arm lengths

- **L1 = 255.0 mm.** ProximalArm.stl puts the shoulder bore at (0, 0) and the elbow bore at (0, −255.00). The circle fit deviates by 0.01 mm.
- **L2 depends on your toolhead.** DistalArm.stl measures 180.5 mm from the elbow bore to the tip, and the tool-mount hole pair sits at 164.1 mm.

With L2 = 164.1 mm the nozzle reaches from 91 mm to 419 mm around the shoulder axis. HALO's central hub has a 54 mm radius, inside the 91 mm limit.
