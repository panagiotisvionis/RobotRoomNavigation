/**
 * Κώδικας Ρομπότ Arduino με 2 Λειτουργίες:
 * 1. Παρακολούθηση διαδρόμου (CORRIDOR_FOLLOW)
 * 2. Πλοήγηση και έξοδος από δωμάτιο (ROOM_NAVIGATION)
 */

#include <Servo.h> // Συμπερίληψη της βιβλιοθήκης Servo για τον έλεγχο του σερβομηχανισμού

// --- Ορισμός PIN για τα μοτέρ ---
#define Lpwm_pin 5 // Ορισμός του pin για το PWM του αριστερού κινητήρα
#define Rpwm_pin 6 // Ορισμός του pin για το PWM του δεξιού κινητήρα
int pinLB = 2;     // Ορισμός του pin για το πίσω αριστερό κινητήρα
int pinLF = 4;     // Ορισμός του pin για το μπροστά αριστερό κινητήρα
int pinRB = 7;     // Ορισμός του pin για το πίσω δεξιό κινητήρα
int pinRF = 8;     // Ορισμός του pin για το μπροστά δεξιό κινητήρα

// --- Ορισμός PIN για αισθητήρα υπερήχων και servo ---
#define TRIG_PIN A1   // Trigger του αισθητήρα υπερήχων
#define ECHO_PIN A0   // Echo του αισθητήρα υπερήχων
#define SERVO_PIN A2  // Pin του σερβομηχανισμού
Servo myservo;       // Δημιουργία αντικειμένου σερβομηχανισμού

// --- Ρυθμίσεις κίνησης και αποστάσεων ---
const int BASE_SPEED = 80;            // Βασική ταχύτητα κίνησης
const int TURN_SPEED = 100;           // Ταχύτητα για στροφή
const int FRONT_MIN_DISTANCE = 15;    // Ελάχιστη απόσταση για να αποφευχθεί εμπόδιο μπροστά
const int DOOR_WIDTH_THRESHOLD = 35;  // Όριο απόστασης για να θεωρηθεί "πόρτα"

// --- Ρυθμίσεις γωνιών για τον σερβομηχανισμό ---
const int SERVO_LEFT = 160;   // Γωνία στροφής προς τα αριστερά
const int SERVO_CENTER = 90;  // Γωνία στο κέντρο (μπροστά)
const int SERVO_RIGHT = 40;   // Γωνία προς τα δεξιά

// --- Ρυθμίσεις καθυστερήσεων ---
const int SERVO_DELAY = 300;         // Καθυστέρηση μετά την κίνηση του servo (ms)
const int MEASURE_DELAY = 60;        // Καθυστέρηση μεταξύ μετρήσεων (ms)
const int CORRIDOR_DEAD_ZONE = 3;    // Νεκρή ζώνη διαφοράς για να θεωρείται στο κέντρο

// --- Λειτουργίες του ρομπότ ---
enum OperationMode {
  CORRIDOR_FOLLOW,   // Λειτουργία παρακολούθησης διαδρόμου
  ROOM_NAVIGATION    // Λειτουργία πλοήγησης και εξόδου από δωμάτιο
};

OperationMode currentMode = ROOM_NAVIGATION; // Επιλογή αρχικής λειτουργίας

// --- Καταστάσεις στη λειτουργία πλοήγησης δωματίου ---
enum RoomState {
  SEEK_WALL,     // Αναζήτηση τοίχου
  FOLLOW_WALL,   // Ακολούθηση τοίχου
  DETECT_DOOR,   // Εντοπισμός πόρτας
  EXIT_ROOM      // Έξοδος από το δωμάτιο
};

RoomState roomState = SEEK_WALL; // Αρχική κατάσταση

// --- Συνάρτηση αρχικοποίησης του ρομπότ ---
void setup() {
  // Ρύθμιση των pins των κινητήρων ως έξοδοι
  pinMode(pinLB, OUTPUT); pinMode(pinLF, OUTPUT);
  pinMode(pinRB, OUTPUT); pinMode(pinRF, OUTPUT);
  pinMode(Lpwm_pin, OUTPUT); pinMode(Rpwm_pin, OUTPUT);

  // Ρύθμιση του αισθητήρα υπερήχων
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Σύνδεση και ευθυγράμμιση του σερβομηχανισμού
  myservo.attach(SERVO_PIN);
  myservo.write(SERVO_CENTER);
  delay(1000); // Μικρή αναμονή για σταθεροποίηση

  // Εκκίνηση σειριακής επικοινωνίας για debugging
  Serial.begin(9600);
  Serial.println("=== ΡΟΜΠΟΤ ΕΤΟΙΜΟ ===");
}

// --- Κύριος βρόχος επανάληψης ---
void loop() {
  if (currentMode == CORRIDOR_FOLLOW) {
    corridorFollowing(); // Λειτουργία παρακολούθησης διαδρόμου
  } else {
    roomNavigation();    // Λειτουργία πλοήγησης δωματίου
  }
  delay(MEASURE_DELAY); // Μικρή καθυστέρηση για σταθερότητα
}

// --- Συνάρτηση μέτρησης απόστασης με αισθητήρα υπερήχων ---
float getDistance() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // Μέγιστο όριο μέτρησης
  if (duration == 0) return 200; // Αν δεν λάβει παλμό, επιστρέφει 200

  float distance = duration / 58.0; // Μετατροπή σε εκατοστά
  delay(10);
  return distance;
}

// --- Μέτρηση απόστασης προς μια συγκεκριμένη κατεύθυνση ---
float measureAtAngle(int angle) {
  myservo.write(angle); // Στρέφει τον servo στη συγκεκριμένη γωνία
  delay(SERVO_DELAY);   // Αναμονή για σταθεροποίηση

  float sum = 0;
  for (int i = 0; i < 3; i++) {
    sum += getDistance(); // Λαμβάνει 3 μετρήσεις
    delay(20);
  }
  return sum / 3.0; // Επιστροφή μέσου όρου
}

// --- Συνάρτηση βασικής κίνησης του ρομπότ ---
void drive(int leftSpeed, int rightSpeed) {
  analogWrite(Lpwm_pin, constrain(leftSpeed, 0, 255));
  analogWrite(Rpwm_pin, constrain(rightSpeed, 0, 255));
  digitalWrite(pinLB, LOW); digitalWrite(pinLF, HIGH);
  digitalWrite(pinRB, LOW); digitalWrite(pinRF, HIGH);
}

// --- Σύντομες συναρτήσεις κίνησης ---
void moveForward(int speed = BASE_SPEED) {
  drive(speed, speed);
}

void turnLeft(int speed = TURN_SPEED) {
  drive(speed - 40, speed + 40); // Στροφή προς τα αριστερά
}

void turnRight(int speed = TURN_SPEED) {
  drive(speed + 40, speed - 40); // Στροφή προς τα δεξιά
}

void back() {
  digitalWrite(pinRB, HIGH); digitalWrite(pinRF, LOW);
  digitalWrite(pinLB, HIGH); digitalWrite(pinLF, LOW);
}

void stopMotors() {
  digitalWrite(pinRB, HIGH); digitalWrite(pinRF, HIGH);
  digitalWrite(pinLB, HIGH); digitalWrite(pinLF, HIGH);
}

// --- Λειτουργία παρακολούθησης διαδρόμου ---
void corridorFollowing() {
  float DL = measureAtAngle(SERVO_LEFT);   // Απόσταση αριστερά
  float DR = measureAtAngle(SERVO_RIGHT);  // Απόσταση δεξιά
  float DM = measureAtAngle(SERVO_CENTER); // Απόσταση μπροστά
  float diff = DL - DR; // Διαφορά αποστάσεων

  Serial.print("DL: "); Serial.print(DL);
  Serial.print(" DR: "); Serial.print(DR);
  Serial.print(" Δ: "); Serial.println(diff);

  if (DL < 10) {
    drive(BASE_SPEED - 15, BASE_SPEED + 15); // Πολύ κοντά αριστερά → στρίβει δεξιά
  } else if (DR < 10) {
    drive(BASE_SPEED + 15, BASE_SPEED - 15); // Πολύ κοντά δεξιά → στρίβει αριστερά
  } else if (abs(diff) < CORRIDOR_DEAD_ZONE) {
    moveForward(BASE_SPEED); // Στο κέντρο → κινείται ευθεία
  } else {
    int adjust = constrain(abs(diff) * 0.6, 5, 25); // Διόρθωση κατεύθυνσης
    if (diff < 0) {
      drive(BASE_SPEED - adjust, BASE_SPEED + adjust); // Πιο κοντά δεξιά
    } else {
      drive(BASE_SPEED + adjust, BASE_SPEED - adjust); // Πιο κοντά αριστερά
    }
  }
}

// --- Λειτουργία πλοήγησης και εξόδου από δωμάτιο ---
void roomNavigation() {
  float frontDist = measureAtAngle(SERVO_CENTER); // Απόσταση μπροστά
  float rightDist = measureAtAngle(SERVO_RIGHT);  // Απόσταση δεξιά

  switch (roomState) {
    case SEEK_WALL:
      if (frontDist < FRONT_MIN_DISTANCE) {
        stopMotors(); delay(500);
        turnRight(); delay(800);
        stopMotors(); delay(300);
        roomState = FOLLOW_WALL; // Τοίχος εντοπίστηκε
      } else {
        moveForward(BASE_SPEED); // Κινείται μέχρι να βρει τοίχο
      }
      break;

    case FOLLOW_WALL:
      if (frontDist < FRONT_MIN_DISTANCE) {
        stopMotors(); delay(300);
        turnLeft(); delay(800);
        stopMotors(); delay(300); // Αποφυγή εμποδίου μπροστά
      } else if (rightDist > DOOR_WIDTH_THRESHOLD) {
        stopMotors(); delay(300);
        roomState = DETECT_DOOR; // Πιθανή πόρτα
      } else {
        if (rightDist < 15) {
          drive(BASE_SPEED + 10, BASE_SPEED - 10); // Πολύ κοντά δεξιά
        } else if (rightDist > 25) {
          drive(BASE_SPEED - 10, BASE_SPEED + 10); // Πολύ μακριά δεξιά
        } else {
          moveForward(BASE_SPEED); // Σταθερή ακολούθηση τοίχου
        }
      }
      break;

    case DETECT_DOOR:
      rightDist = measureAtAngle(SERVO_RIGHT);
      if (rightDist > DOOR_WIDTH_THRESHOLD) {
        turnRight(); delay(600); stopMotors(); delay(300);
        roomState = EXIT_ROOM; // Πόρτα επιβεβαιώθηκε
      } else {
        roomState = FOLLOW_WALL; // Ψευδής ανίχνευση
      }
      break;

    case EXIT_ROOM:
      moveForward(BASE_SPEED); delay(2000); // Προχώρα ευθεία για 2 δευτερόλεπτα
      stopMotors();
      Serial.println(">>> ΕΠΙΤΥΧΗΣ ΕΞΟΔΟΣ ΑΠΟ ΤΟ ΔΩΜΑΤΙΟ <<<");
      while (true) delay(1000); // Σταμάτησε το πρόγραμμα
      break;
  }
}
