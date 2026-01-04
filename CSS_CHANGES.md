# CSS Changes for 480x320 Display

## Before vs After Comparison

### Header

```
BEFORE: padding: 12px 16px; font-size: 20px;
AFTER:  padding: 8px 12px;  font-size: 16px;  (14px on mobile)
```

### Cadence/Resistance Values

```
BEFORE: font-size: 72px;
AFTER:  font-size: 56px;  (48px on screens < 500px)
```

### Large Value (if used)

```
BEFORE: font-size: 90px;
AFTER:  font-size: 64px;  (56px on screens < 500px)
```

### Units

```
BEFORE: font-size: 20px;
AFTER:  font-size: 18px;  (16px on screens < 500px)
```

### Spacing

```
BEFORE: gap: 8px;  padding: 8px;
AFTER:  gap: 6px;  padding: 6px;
```

### Badges

```
BEFORE: padding: 6px 12px; font-size: 14px;
AFTER:  padding: 5px 10px; font-size: 12px;
```

## Layout on 480x320

```
┌─────────────────────────────────────────────────┐
│ Gym Bike                                    ⚙  │ ← 8px padding, 16px font
├─────────────────────────────────────────────────┤
│ ┌─────────────────────────────────────────────┐ │
│ │ [Stopped]                      00:00        │ │ ← Workout status
│ └─────────────────────────────────────────────┘ │
│                                                  │
│ ┌───────────────────┐ ┌─────────────────────┐  │
│ │   CADENCE         │ │   RESISTANCE        │  │
│ │                   │ │                     │  │
│ │      85 rpm       │ │       45 %          │  │ ← 48-56px values
│ │                   │ │                     │  │
│ └───────────────────┘ └─────────────────────┘  │
│                                                  │
│ ┌─────────────────────────────────────────────┐ │
│ │ Mode: [Cadence]    Target: 90 rpm          │ │ ← Control info
│ └─────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────┘
       480px wide × 320px tall
```

## Key Improvements

### 1. Responsive Design

Added media query for small screens:

```css
@media (max-width: 500px) {
  .card-value {
    font-size: 48px !important;
  }
  .card-value.large {
    font-size: 56px !important;
  }
  .card-unit {
    font-size: 16px !important;
  }
  header h1 {
    font-size: 14px;
  }
}
```

### 2. Compact Layout

- Reduced all padding and margins by 25-33%
- Tighter line-height for better space usage
- Smaller badge sizes for status indicators

### 3. Better Readability

- Still large enough to read from a distance
- Maintained contrast ratios
- Preserved color-coded status indicators

### 4. Flexible Grid

Grid system adapts to screen size:

- Small screens (< 768px): Single column
- Larger screens: Two columns
- Always uses full available space

## Font Size Progression

```
Mobile (< 500px):
Header: 14px → Values: 48px → Large: 56px → Units: 16px

Desktop (480x320):
Header: 16px → Values: 56px → Large: 64px → Units: 18px

Original (> 768px):
Header: 20px → Values: 72px → Large: 90px → Units: 20px
```

## Visual Hierarchy Maintained

1. **Primary Data** (Cadence/Resistance): Largest (48-56px)
2. **Secondary Data** (Time, Status): Medium (20-28px)
3. **Labels**: Small (10-14px)
4. **Meta Info**: Smallest (9-10px)

The hierarchy ensures users can quickly see the most important information (current cadence and resistance) even on the small display.
