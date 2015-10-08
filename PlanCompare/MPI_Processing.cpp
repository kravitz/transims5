//*********************************************************
//	MPI_Processing.cpp - MPI data transfer
//*********************************************************

#include "PlanCompare.hpp"

//---------------------------------------------------------
//	MPI_Processing
//---------------------------------------------------------

void PlanCompare::MPI_Processing (void)
{
#ifdef MPI_EXE
	int i, lvalue [7], rank;

	if (Master ()) {
		Write (1);

		for (i=1; i < MPI_Size (); i++) {

			rank = Show_MPI_Message ();

			//---- retrieve the data buffer ----

			Get_MPI_Buffer (rank);
			
			if (mpi_buffer.Size () == 0) continue;

			//---- retrieve the plan statistics

			mpi_buffer.Get_Data (lvalue, sizeof (lvalue));

			plan_file->Add_Files (lvalue [0]);
			plan_file->Add_Records (lvalue [1]);
			plan_file->Add_Households (lvalue [2]);
			plan_file->Add_Persons (lvalue [3]);
			plan_file->Add_Tours (lvalue [4]);
			plan_file->Add_Trips (lvalue [5]);
			num_trips += lvalue [6];

			//---- retrieve the compare statistics

			mpi_buffer.Get_Data (lvalue, sizeof (lvalue));

			compare_file.Add_Files (lvalue [0]);
			compare_file.Add_Records (lvalue [1]);
			compare_file.Add_Households (lvalue [2]);
			compare_file.Add_Persons (lvalue [3]);
			compare_file.Add_Tours (lvalue [4]);
			compare_file.Add_Trips (lvalue [5]);
			num_compare += lvalue [6];

			//---- selection map data ----

			selected.UnPack (mpi_buffer);

			//---- time difference data ----

			if (time_diff.Active_Flag ()) {
				Difference_Data temp;
				temp.Replicate (time_diff);

				temp.UnPack (mpi_buffer);
				time_diff.Merge_Data (temp);
			}

			//---- cost difference data ----

			if (cost_diff.Active_Flag ()) {
				Difference_Data temp;
				temp.Replicate (cost_diff);

				temp.UnPack (mpi_buffer);
				cost_diff.Merge_Data (temp);
			}

			//---- time gap data ----

			if (time_gap.Active_Flag ()) {
				Trip_Gap_Data temp;
				temp.Replicate (time_gap);

				temp.UnPack (mpi_buffer);
				time_gap.Merge_Data (temp);
			}

			//---- cost gap data ----

			if (cost_gap.Active_Flag ()) {
				Trip_Gap_Data temp;
				temp.Replicate (cost_gap);

				temp.UnPack (mpi_buffer);
				cost_gap.Merge_Data (temp);
			}

		}
		Show_Message (1);

		plan_file->Close ();
		compare_file.Close ();

	} else {	//---- slave ----

		if (Partition_Range ()) {
			Send_MPI_Message (String ("Slave %d Read %ss %d..%d") % MPI_Rank () % plan_file->File_Type () % First_Partition () % Last_Partition ());
		} else {
			Send_MPI_Message (String ("Slave %d Read %s %d") % MPI_Rank () % plan_file->File_Type () % First_Partition ());
		}

		//---- send plan statistics ----

		lvalue [0] = plan_file->Num_Files ();
		lvalue [1] = plan_file->Num_Records ();
		lvalue [2] = plan_file->Num_Households ();
		lvalue [3] = plan_file->Num_Persons ();
		lvalue [4] = plan_file->Num_Tours ();
		lvalue [5] = plan_file->Num_Trips ();
		lvalue [6] = num_trips;
		plan_file->Close ();

		mpi_buffer.Data (lvalue, sizeof (lvalue));

		//---- send compare statistics ----

		lvalue [0] = compare_file.Num_Files ();
		lvalue [1] = compare_file.Num_Records ();
		lvalue [2] = compare_file.Num_Households ();
		lvalue [3] = compare_file.Num_Persons ();
		lvalue [4] = compare_file.Num_Tours ();
		lvalue [5] = compare_file.Num_Trips ();
		lvalue [6] = num_compare;
		compare_file.Close ();

		mpi_buffer.Add_Data (lvalue, sizeof (lvalue));

		//---- selection map data ----

		selected.Pack (mpi_buffer);

		//---- time difference data ----

		if (time_diff.Active_Flag ()) {
			time_diff.Pack (mpi_buffer);
		}

		//---- cost difference data ----

		if (cost_diff.Active_Flag ()) {
			cost_diff.Pack (mpi_buffer);
		}

		//---- time gap data ----

		if (time_gap.Active_Flag ()) {
			time_gap.Pack (mpi_buffer);
		}

		//---- cost gap data ----

		if (cost_gap.Active_Flag ()) {
			cost_gap.Pack (mpi_buffer);
		}

		//---- send the data buffer ----

		Send_MPI_Buffer ();

		if (!new_plan_flag && !select_parts) Exit_Stat (DONE);
	}
#endif
}
